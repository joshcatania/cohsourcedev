#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <stddef.h>
#include "patchapply.h"
#include "filechecksum.h"
#include "patchfileutils.h"
#include "utils.h"
#include "patchheader.h"
#include "zlib.h"
#include "bindiff.h"
#include "error.h"
#include "mathutil.h"
#include "crypt.h"
#include "patchui.h"
#include "StashTable.h"
#include "wininclude.h"
#include <psapi.h>

static int corrupt_file_made;
extern int g_COV;

void *tracking_malloc(size_t amt)
{
	PROCESS_MEMORY_COUNTERS pps;
	BOOL ret;

	if (amt > 1024*1024)
		printf("Malloc: %u\n", amt);

	ret = GetProcessMemoryInfo(GetCurrentProcess(), &pps, sizeof(PROCESS_MEMORY_COUNTERS));

	if (ret)
	{
		if (pps.WorkingSetSize > 512*1024*1024)
		{
			printf("WSS: %u\n", pps.WorkingSetSize);
		}
	}

	return malloc(amt);
}

#undef malloc
#define malloc(AMT) tracking_malloc(AMT)

static U32 getPackSize(PigCache *cache,ImageCheck *image,FileCheck *file)
{
	PigFileHeader *pfh;

	if (!file->is_compressed)
		return 0;
	pfh = getPfh(cache,image->fname,file->name,file->pig_name);
	if (!pfh)
		return 0;
	return pfh->pack_size;
}

static int readZipData(GzStream *gz,FileCheck *file,void **zip_data_p,void **unpack_data_p)
{
	U32		ret;
	U8		*unpack_data,*pack_data=0;

	unpack_data = malloc(file->data_size);
	if (file->data_pack_size)
	{
		pack_data = malloc(file->data_pack_size);
		ret = gzGetBytes(gz,file->data_pack_size,pack_data,1);
		assert(ret);
		uncompress(unpack_data,&file->data_size,pack_data,file->data_pack_size);
	}
	else
	{
		ret = gzGetBytes(gz,file->data_size,unpack_data,1);
		assert(ret);
	}
	checksumMem(unpack_data,file->data_size,file->full.values);
	if (zip_data_p)
		*zip_data_p = pack_data;
	else
		free(pack_data);
	if (unpack_data_p)
		*unpack_data_p = unpack_data;
	else
		free(unpack_data);
	return 1;
}

static U8 *loadPatchedData(PigCache *cache,GzStream *gz,FileCheck *file,char *image_dir,U32 *size_p,U32 *pack_size_p,int just_read)
{
	U32		size,ret,pack_size_dump;
	U8		*data;

	if (!pack_size_p)
		pack_size_p = &pack_size_dump;
	*pack_size_p = 0;
	if (file->mod)
	{
		U8		*src_data,*patch_data;
		U32		src_size;

		ret = readZipData(gz,file,0,&patch_data);
		assert(ret);
		if (just_read)
			return patch_data;
		src_data = loadFileData(cache,image_dir,file->name,file->pig_name,&src_size,0);
		size = bindiffApplyPatch(src_data,patch_data,&data);
		free(src_data);
		free(patch_data);
		file->data_pack_size = 0;
	}
	else if (file->added)
	{
		if (file->pig_name)
		{
			void *zip_data;

			ret = readZipData(gz,file,&zip_data,&data);
			if (zip_data)
			{
				free(data);
				data = zip_data;
				*pack_size_p = file->data_pack_size;
			}
		}
		else
			ret = readZipData(gz,file,0,&data);
		size = file->data_size;
		assert(ret);
		if (just_read)
			return data;
	}
	else
	{
		if (just_read)
			return 0;
		if ((data = loadFileData(cache,image_dir,file->name,file->pig_name,&size,1)))
		{
			*pack_size_p = size;
			size = file->full.size;
		}
		else
			data = loadFileData(cache,image_dir,file->name,file->pig_name,&size,0);
	}
	*size_p = size;
	return data;
}

int safeWritePigFile(char *pig_name,NewPigEntry *entries,int entry_count,ImageCheck *dst_image,char *dst_dir,U32 timestamp)
{
	char	fname[MAX_PATH],tempname[MAX_PATH];

	sprintf(fname,"%s/%s",dst_dir,pig_name);
	sprintf(tempname,"%s/%s",dst_dir,"tempfile");
	safeMkDirTree(fname);
	if (entry_count)
	{
		Checksum	check;
		FileCheck	*dst;

		for(;;)
		{
			safeDeleteFile(tempname);
			if (pigCreateFromMem(entries,entry_count,tempname, PIG_VERSION))
				break;
			fileMsgAlert("ErrCouldntCreateFile",tempname);
		}
		checksumFile(tempname,&check,0);
		stashFindPointer(dst_image->filenames,pig_name, &dst);
		assert(dst);
		if (!checksumMatch(&dst->full,&check))
		{
			fileMsgAlert("ErrFileChecksum",dst->name);
			corrupt_file_made = 1;
			return 0;
		}
		safeDeleteFile(fname);
		if (rename(tempname,fname) != 0)
		{
			msgAlertFatal("ErrRenameFailed",tempname,fname);
		}
		safeFileSetTimestamp(fname, timestamp);
		chmod(fname,_S_IREAD);
	}
	else
		safeDeleteFile(fname);
	return 1;
}

int createPig(PigCache *cache,ImageCheck *mod_image,ImageCheck *dst_image,char *pig_name,char *image_dir,char *dst_dir,GzStream *gz,int write_file,int src_file_valid)
{
	int			i,ret=1;
	FileCheck	*mod_file;
	NewPigEntry	*entries=0,*entry;
	int			entry_count=0,entry_max=0;
	U32			timestamp=0;
	char		fname[MAX_PATH];

	for(i=0;i<mod_image->file_count;i++)
	{
		mod_file = mod_image->files[i];
		if (stricmp(mod_file->name,pig_name)==0)
			timestamp = mod_file->timestamp;
		if (!mod_file->pig_name || stricmp(mod_file->pig_name,pig_name)!=0)
			continue;
		//mod_file->visited = 1;
		if (mod_file->del)
			continue;
		entry = dynArrayAdd(&entries,sizeof(entries[0]),&entry_count,&entry_max,1);
		entry->fname		= mod_file->name;
		entry->timestamp	= mod_file->timestamp;
		entry->size			= mod_file->data_size;
		entry->data			= loadPatchedData(cache,gz,mod_file,image_dir,&entry->size,&entry->pack_size,!write_file);
		if ( (((src_file_valid && !mod_file->mod) || mod_file->added) && !entry->pack_size) ||
			// hack: 
			// at the time of this comment, lightmaps are a special case.  they should never
			// be zipped
			strEndsWith(entry->fname, ".lightmapIndex") || strEndsWith(entry->fname, ".lightmap") )
			// end hack
			entry->dont_pack = 1;
		memcpy(entry->checksum,mod_file->full.values,sizeof(entry->checksum));
	}
	closeOpenPigs(cache);
	if (write_file)
		ret = safeWritePigFile(pig_name,entries,entry_count,dst_image,dst_dir,timestamp);
	sprintf(fname,"%s/%s",dst_dir,pig_name);
	xferStatsUpdate(safeFileSize(fname));

	for(i=0;i<entry_count;i++)
		free(entries[i].data);
	free(entries);
	return ret;
}

static FILE *unpackPatch(char *patch_name,char **cmd_mem,char **manifest)
{
	PatchHeader patch;
	U8			*zip,*patch_u8 = (U8*)&patch;
	FILE		*file;
	U32			unpack_size_32;

	file = fopen(patch_name,"rb");
	if (!file)
		return 0;
	fread(&patch,1,sizeof(patch),file);
	zip = malloc(MAX(patch.cmd.size,patch.manifest.size));
	fseek(file,(U32)patch.cmd.pos,0);
	fread(zip,1,patch.cmd.size,file);
	*cmd_mem = calloc(patch.cmd.unpack_size+1,1);
	unpack_size_32 = patch.cmd.unpack_size;
	uncompress(*cmd_mem,&unpack_size_32,zip,patch.cmd.size);
	patch.cmd.unpack_size = unpack_size_32;
	fseek(file,(U32)patch.manifest.pos,0);
	fread(zip,1,patch.manifest.size,file);
	*manifest = calloc(patch.manifest.unpack_size+1,1);
	unpack_size_32 = patch.manifest.unpack_size;
	uncompress(*manifest,&unpack_size_32,zip,patch.manifest.size);
	patch.manifest.unpack_size = unpack_size_32;
	fseek(file,sizeof(patch),0);
	free(zip);
	return file;
}

static void processPatchCommands(ImageCheck *client_image,ImageCheck *mod_image,char *s)
{
	char		*args[100],*cmd,*fname,*pig_name;
	U32			count,patch_size,timestamp=0,data_pos=0;
	int			i,unpack_size;
	FileCheck	*src_file,*mod_file;

	while(count = tokenize_line(s,args,&s))
	{
		unpack_size = 0;
		patch_size = 0;
		cmd = args[0];
		fname = args[2];
		pig_name = args[1];
		if (count >= 5)
		{
			patch_size = strtoul(args[3],0,10);
			timestamp = strtoul(args[4],0,10);
			if (count >= 6)
				unpack_size = strtoul(args[5],0,10);
		}
		stashFindPointer(client_image->filenames,fname, &src_file);
		stashFindPointer(mod_image->filenames,fname, &mod_file);
		if (!mod_file)
			mod_file = checksumAddFile(mod_image,fname,0,1);
		if (stricmp(pig_name,"FILE")!=0)
			mod_file->pig_name = strdup(pig_name);
		if (stricmp(cmd,"Del")==0)
		{
			mod_file->del = 1;
		}
		else if (stricmp(cmd,"Add")==0)
		{
			mod_file->data_pos = data_pos;
			mod_file->data_size = unpack_size;
			mod_file->data_pack_size = patch_size;
			mod_file->timestamp = timestamp;
			mod_file->added = 1;
		}
		else if (stricmp(cmd,"Mod")==0)
		{
			mod_file->data_pos = data_pos;
			mod_file->data_size = unpack_size;
			mod_file->data_pack_size = patch_size;
			mod_file->timestamp = timestamp;
			mod_file->mod = 1;
		}
		else if (stricmp(cmd,"Time")==0)
		{
			mod_file->timestamp = timestamp;
		}
		else
			msgAlertFatal("ErrUnknownPatchCommand");
		mod_file->in_patch = 1;
		data_pos += patch_size;
	}
	for(i=0;i<client_image->file_count;i++)
	{
		src_file = client_image->files[i];
		if (!stashFindPointer(mod_image->filenames,src_file->name, NULL))
		{
			mod_file = checksumAddFile(mod_image,src_file->name,0,1);
			fname = mod_file->name;
			*mod_file = *src_file;
			mod_file->name = fname;
			if (mod_file->pig_name)
				mod_file->pig_name = strdup(mod_file->pig_name);
		}
	}
}

U32 patchApplyByteCount(ImageCheck *mod_image,ImageCheck *dst_image)
{
	U32			i,byte_count=0;
	FileCheck	*mod_file,*dst_file;

	for(i=0;i<(U32)mod_image->file_count;i++)
	{
		mod_file = mod_image->files[i];
		if (mod_file->pig_name || !mod_file->in_patch || mod_file->del)
			continue;
		stashFindPointer(dst_image->filenames,mod_file->name, &dst_file);
		byte_count += dst_file->full.size;
	}
	return byte_count;
}

int createPatchedFiles(ImageCheck *client_image,ImageCheck *mod_image,ImageCheck *dst_image,FILE *data_file,char *image_dir,char *dst_dir)
{
	int			i,write_file,ret=0;
	U8			*data;
	U32			size;
	char		fname[MAX_PATH];
	FileCheck	*dst_file,*mod_file,*src_file;
	PigCache	cache = {0};
	GzStream	gz_stream;

	gzStreamDecompressInit(&gz_stream,data_file);
	for(i=0;i<mod_image->file_count;i++)
	{
		mod_file = mod_image->files[i];
		if (/*mod_file->visited ||*/ mod_file->pig_name || !mod_file->in_patch)
			continue;
		stashFindPointer(client_image->filenames,mod_file->name, &src_file);
		if (!src_file || src_file->full_status != CHECKSUM_MATCHES_PATCH)
			write_file = 1;
		else
			write_file = 0;
		sprintf(fname,"%s/%s",dst_dir,mod_file->name);
		if (mod_file->del)
		{
			safeDeleteFile(fname);
			continue;
		}
		if (strEndsWith(mod_file->name,".pigg"))
		{
			int src_file_valid=0;

			if (strstri(mod_file->name,"sound.pigg"))
				printf("");
			if (src_file && src_file->full_status)
				src_file_valid = 1;
			if (!createPig(&cache,mod_image,dst_image,mod_file->name,image_dir,dst_dir,&gz_stream,write_file,src_file_valid))
				goto err_exit;
		}
		else
		{
			safeMkDirTree(fname);
			data = loadPatchedData(&cache,&gz_stream,mod_file,image_dir,&size,0,!write_file);
			if (write_file)
			{
				stashFindPointer(dst_image->filenames,mod_file->name, &dst_file);
				assert(dst_file);
				if (!safeOverwriteFileAndChecksum(fname,"wb",data,size,&dst_file->full,0))
				{
					corrupt_file_made = 1;
					goto err_exit;
				}
				safeFileSetTimestamp(fname, mod_file->timestamp);
			}
			free(data);
			xferStatsUpdate(safeFileSize(fname));
		}
		//mod_file->visited = 1;
		closeOpenPigs(&cache);
	}
	ret = 1;
err_exit:
	gzStreamFree(&gz_stream);
	fclose(gz_stream.file);
	return ret;
}

static void copyUnpatchedFiles(ImageCheck *dst_image,ImageCheck *mod_image,char *src_dir,char *dst_dir)
{
	FileCheck	*dst_file,*mod_file;
	char		src_fname[MAX_PATH],fname[MAX_PATH];
	int			i;

	for(i=0;i<dst_image->file_count;i++)
	{
		dst_file = dst_image->files[i];
		sprintf(fname,"%s/%s",dst_dir,dst_file->name);
		stashFindPointer(mod_image->filenames,dst_file->name, &mod_file);
		if (!mod_file || !mod_file->in_patch)
		{
			sprintf(src_fname,"%s/%s",src_dir,dst_file->name);
			safeCopyFile(src_fname,fname);
			safeFileSetTimestamp(fname, dst_file->timestamp);
			xferStatsUpdate(safeFileSize(fname));
		}
	}
}

int applyPatch(char *project_name,char *patch_name,char *checksum_name,char *src_dir,char *dst_dir,int has_checksum)
{
	int			success = 0,diff_dirs;
	U8			*cmd_mem;
	char		*manifest,dst_checksum_name[MAX_PATH];
	ImageCheck	dst_image = {0},client_image = {0},mod_image = {0};
	FILE		*patch_file;

	corrupt_file_made = 0;
	diff_dirs = stricmp(src_dir,dst_dir);
	printf( "Applying %s project patch %s to %s...", project_name, patch_name, dst_dir );
	if (!(patch_file = unpackPatch(patch_name,&cmd_mem,&manifest)))
	{
		printf( "Failed to unpack patch\n" ) ;
		goto fail;
	}
	if (has_checksum && !checksumLoad(checksum_name,&client_image))
	{
		printf( "Failed to load checksum\n" );
		goto fail;
	}
	checksumLoadFromMem(manifest,&dst_image,1);
	// if they are running a cov updater and trying to patch to a version below 10, dont let them
	if ( g_COV && getUpdateVersion(dst_image.build_name) < 10 )
	{
		printf( "Trying to patch CoV to a version below 10\n" );
		success = -2;
		goto fail;
	}
	checksumVerify(src_dir,&client_image,&dst_image,CHECKSUM_NEVER,0);
	xferStatsInit("StatsApplying",0,client_image.bytecount);
	checksumOpenPigs(&client_image,src_dir,0);
	processPatchCommands(&client_image,&mod_image,cmd_mem);
	xferStatsInit("StatsApplying",0,diff_dirs ? client_image.bytecount : patchApplyByteCount(&mod_image,&dst_image));
	if (!createPatchedFiles(&client_image,&mod_image,&dst_image,patch_file,src_dir,dst_dir))
	{
		printf( "Failed to create patched files\n" );
		if (!strEndsWith(patch_name,".majorpatch"))
			safeDeleteFile(patch_name);
		success = -1;
		goto fail;
	}
	if (diff_dirs)
		copyUnpatchedFiles(&dst_image,&mod_image,src_dir,dst_dir);
	xferStatsFinish();

	if (!strEndsWith(patch_name,".majorpatch"))
		safeDeleteFile(patch_name);

	if (checksumVerify(dst_dir,&dst_image,0,0,0))
	{
		sprintf(dst_checksum_name,"%s/%s.checksum",dst_dir,project_name);
		safeOverwriteFileAndChecksum(dst_checksum_name,"wb",manifest,strlen(manifest),0,1);
		success = 1;
	}
fail:
	xferStatsFinish();
	checksumFree(&mod_image);
	checksumFree(&dst_image);
	checksumFree(&client_image);
	if (success != 1 && !corrupt_file_made)
		success == -2 ? msgAlertUpdater("ErrPatchCovToCoh") : msgAlertUpdater("ErrPatchFailed");
	removeEmptyDirs(dst_dir);

	if ( success == -2 )
		exit(1);
	return success;
}


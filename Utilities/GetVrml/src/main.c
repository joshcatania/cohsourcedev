#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilitieslib/stdtypes.h>
#include "tree.h"
#include "vrml.h"
#include "output.h"
#include "seq/tricks.h"
#include "gridcoll/grid.h"
#include "texsort.h"
#include <utilitieslib/utils/file.h>
#include "geo.h"
#include <utilitieslib/utils/error.h>
#include <utilitieslib/utils/utils.h>
#include "tree.h"
#include <utilitieslib/assert/assert.h>
#include <utilitieslib/utils/piglib.h>
#include <utilitieslib/utils/FolderCache.h>
#include <utilitieslib/utils/fileutil.h>
#include <utilitieslib/utils/timing.h>
#include <utilitieslib/components/StashTable.h>
#include "cmdparse/cmdcommon.h"
#include "seq/AutoLOD.h"
#include <utilitieslib/utils/winutil.h>
#include <utilitieslib/utils/sysutil.h>
#include <utilitieslib/components/sharedmemory.h>

#include <io.h>
#include <conio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utilitieslib/components/earray.h>

GlobalState global_state;

static StashTable vrmls_by_trick_file = 0;
static StashTable vrmls_by_lod_file = 0;

typedef struct _VrmlList
{
    char **vrmls;
} VrmlList;

int no_check_out, no_lods = 0, g_force_rebuild;
char g_output_dir[MAX_PATH];
int do_meshMend = 1, no_scan = 0;
int atlas_roundtrip = 0;
static int atlas_offline = 0;
int force_ReLOD_Always = 1; // per jay, always answers yes so just force it

#define MAX_FNAMES 10000

//given a directory, fills 'fnames' with the names of all the files in that directory and kids of type
//'extension' and records fname_count.  if just a 'extension' file is given, it just writes that into fnames
static void getFileNames(char * fname, char** fnames, int * cur_fname_count, char * extension )
{
    struct _finddata_t fileinfo;
    int        handle,test,len, extlen;
    char    buf[1200];

    len = strlen(fname);
    extlen = strlen(extension);

    if (fname[strlen(fname)-1]=='/')
        fname[strlen(fname)-1]='\0';
    sprintf(buf,"%s/*",fname);
    for(test = handle = _findfirst(buf,&fileinfo);test >= 0;test = _findnext(handle,&fileinfo))
    {
        if (fileinfo.name[0] == '.' || fileinfo.name[0] == '_')
            continue;
        sprintf(buf, "%s/%s", fname, fileinfo.name);
        len = strlen(buf);
        if ( len > extlen && ( strcmpi(buf + len - extlen, extension ) == 0 ) )
            fnames[(*cur_fname_count)++] = strdup(buf);
        if (fileinfo.attrib & _A_SUBDIR)
            getFileNames(buf, fnames, cur_fname_count, extension);
        assert(*cur_fname_count < MAX_FNAMES);
    }
    _findclose(handle);
}

// Read a text file which contains a list of files, one per line, and add files which match the given extension
//    to the list.
static int getFileNames_fromList(char* fileListPath, char** fnames, char* extension)
{
    int name_count = 0, len;
    char buffer[1024];
    FILE* fp;

    fp = fopen(fileListPath, "rt");
    if( !fp ) {
        printf("Unable to find file list \"%s\".  No files will be processed\n", fileListPath);
        return 0;
    }

    // Extract all directory names and add them to the string table.
    while( fgets(buffer,sizeof(buffer), fp) )
    {
        len = strlen(buffer) - 1;
        if( buffer[0] == '#' )
            continue; // allow comments
        while( len > 0 && (buffer[len] == '\n'|| buffer[len] == '\r' || buffer[len] == '\t' || buffer[len] == ' ') )
            buffer[len--] = 0; // remove trailing spaces
        if( len <= 0 )
            continue;

        forwardSlashes(buffer);
        if( !strEndsWith(buffer, extension) )
        {
            printf("Skipping filelist entry \"%s\" because it doesn't match extension \"%s\"\n", buffer, extension);
            continue;
        }

        // report error if file doesn't exist, since we are going to process next and that will fatal error if
        //    file is missing
        if( !fileExists(buffer) ) {
            Errorf("List contains a file that does not exist, skipping: \"%s\"", buffer);
            continue;
        }

        fnames[name_count++] = strdup(buffer);
        assert(name_count < MAX_FNAMES);
    }

    return name_count;
}

//takes (datasrc)/object_library... for a file and converts it to (target)/object_libray...
//otherwise keeping the file structure
static void srcToData(char *src_name,char *data_name)
{
char    *s;

    s = strstri(src_name,"object_library");
    if (g_output_dir[0]) {
        sprintf(data_name,"%s/%s",g_output_dir,s);
    } else {
        sprintf(data_name,"%s/%s",fileDataDir(),s);
    }
    forwardSlashes(data_name);
}

static int isLegacy2(GeoLoadData *gld)
{
    int j;
    if (!gld || !gld->modelheader.model_count)
    {
        // Error loading geo file.  File may not exist, assume it is not legacy.
        return 0;
    }

    if (gld->file_format_version > 5 && !gld->lod_infos)
        return 0;

    for (j = 0; j < gld->modelheader.model_count; j++)
    {
        Model *model = gld->modelheader.models[j];
        if (model->trick && model->trick->info && model->trick->info->auto_lod)
            return 0;
        if (model->lod_info && !(model->lod_info->lods[0]->flags & LOD_LEGACY))
            return 0;
    }

    return 1;
}

static int isLegacy(char *geo_name, char *vrml_name)
{
#if 0 // fpe disabled, we always allow re-lod'ing
    char buffer[2048];
    GeoLoadData *gld;

    trickSetExistenceErrorReporting(0);
    geoSetExistenceErrorReporting(0);
    gld = geoLoad(geo_name, LOAD_HEADER, GEO_USED_BY_WORLD);
    trickSetExistenceErrorReporting(1);
    geoSetExistenceErrorReporting(1);

    if (!isLegacy2(gld))
        return 0;

    // ask user if this should use AutoLOD or be legacy
    sprintf(buffer, "File %s has been exported using the old exporter. Auto-generating LODs could potentially cause visual problems with the object.  Do you wish to use the AutoLOD system on this object?", vrml_name);
    return MessageBox(0, buffer, "GetVrml", MB_YESNO) == IDNO;
#else
    return 0; // not legacy means ok to create lods for player_library.  Jay always uses this option, not sure why choice was required here.
#endif
}

static int closeEnoughLODs(ModelLODInfo *linfo1, ModelLODInfo *linfo2, int is_player_library)
{
    int i;

    if (linfo1->bits.is_automatic != linfo2->bits.is_automatic)
        return 0;

    if (eaSize(&linfo1->lods) != eaSize(&linfo2->lods))
        return 0;

    for (i = 0; i < eaSize(&linfo1->lods); i++)
    {
        if (linfo1->lods[i]->flags != linfo2->lods[i]->flags)
            return 0;
        if (!nearSameF32(linfo1->lods[i]->max_error, linfo2->lods[i]->max_error))
            return 0;
        if (!is_player_library)
        {
            if (!nearSameF32(linfo1->lods[i]->lod_near, linfo2->lods[i]->lod_near))
                return 0;
            if (!nearSameF32(linfo1->lods[i]->lod_far, linfo2->lods[i]->lod_far))
                return 0;
            if (!nearSameF32(linfo1->lods[i]->lod_nearfade, linfo2->lods[i]->lod_nearfade))
                return 0;
            if (!nearSameF32(linfo1->lods[i]->lod_farfade, linfo2->lods[i]->lod_farfade))
                return 0;
        }
    }

    return 1;
}

static int doReLODWarningCheck(char *vrml_name, char *geo_name, int is_player_library)
{
    ModelLODInfo *lod_desired;
    int j;
    GeoLoadData *gld;
    Vec3 dv;
    int is_legacy, show_warning = 0;

    trickSetExistenceErrorReporting(0);
    geoSetExistenceErrorReporting(0);
    gld = geoLoad(geo_name, LOAD_HEADER, GEO_USED_BY_WORLD);
    trickSetExistenceErrorReporting(1);
    geoSetExistenceErrorReporting(1);

    if (!gld || gld->file_format_version > 5)
        return 1;

    is_legacy = isLegacy2(gld);

    for (j=0; j<gld->modelheader.model_count; j++)
    {
        Model *model = gld->modelheader.models[j];
        int lod_num = getAutoLodNum(model->name);
        if (lod_num > 0)
            continue;

        subVec3(model->max, model->min, dv);
        lod_desired = lodinfoFromObjectName(is_player_library?getPlayerLibraryDefaultLODs():0, model->name, model->filename, lengthVec3(dv), is_legacy, model->tri_count, 0);

        if ((lod_desired->bits.is_from_trick || lod_desired->bits.is_from_default) && (eaSize(&lod_desired->lods) > 1 || lod_desired->lods[0]->max_error))
        {
            show_warning = 1;
            break;
        }
    }

    if (!no_check_out && show_warning)
    {
        char buffer[1024];
        sprintf(buffer, "WARNING: %s was previously LODed with a different version of the LODifier, the LODs may look different.  Proceed?", vrml_name);
        if(force_ReLOD_Always) {
            strcat(buffer, "==> forcing ReLOD since force_ReLOD_Always option is set.\n");
            printf(buffer);
            return 1;
        } else {
            return MessageBox(0, buffer, "GetVrml", MB_YESNO) == IDYES;
        }
    }

    return 1;
}

static int lodParamsChanged(char *geo_name, char *vrml_name, int is_player_library)
{
    ModelLODInfo *lod_desired;
    int j;
    GeoLoadData *gld;
    Vec3 dv;
    int is_legacy;

    trickSetExistenceErrorReporting(0);
    geoSetExistenceErrorReporting(0);
    gld = geoLoad(geo_name, LOAD_HEADER, GEO_USED_BY_WORLD);
    trickSetExistenceErrorReporting(1);
    geoSetExistenceErrorReporting(1);

    if (!gld)
    {
        // Error loading geo file.  Not sure what happened, but LOD parameters could not be loaded from geo, so assume they have not changed.
        return 0;
    }

    if (gld->file_format_version >= 7 && !is_player_library)
        return 0; // nothing to do here, all work is done by game

    is_legacy = isLegacy2(gld);

    for (j=0; j<gld->modelheader.model_count; j++)
    {
        Model *model = gld->modelheader.models[j];
        int lod_num = getAutoLodNum(model->name);
        if (lod_num > 0)
            continue;

        subVec3(model->max, model->min, dv);
        lod_desired = lodinfoFromObjectName(is_player_library?getPlayerLibraryDefaultLODs():0, model->name, model->filename, lengthVec3(dv), is_legacy, model->tri_count, 0);

        if (!model->lod_info && lod_desired->bits.is_from_default)
        {
            // no lod info in geo, defaults desired
            return 0;
        }

        if (!model->lod_info && (lod_desired->bits.is_from_trick || !lod_desired->has_bits))
        {
            // no lod info in geo, but there are lod parameters specified by the trick or lod info file
            return 1;
        }

        if (model->lod_info)
        {
            if (is_player_library || !lod_desired->bits.is_automatic)
            {
                if (!closeEnoughLODs(model->lod_info, lod_desired, is_player_library))
                {
                    // new lod parameters are different from those in the geo
                    return 1;
                }
            }
            else if (lod_desired->force_auto)
            {
                return 1;
            }
            else if (lod_desired->bits.is_automatic)
            {
                float diff = model->lod_info->lods[eaSize(&model->lod_info->lods)-1]->lod_far - lod_desired->lods[eaSize(&lod_desired->lods)-1]->lod_far;
                if (fabs(diff) > 1)
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

static void addVrmlToTrickHash(char *vrml_name, char *geo_name, char *trick_file_name)
{
    VrmlList *vl;

    if (!vrmls_by_trick_file)
        vrmls_by_trick_file = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);

    if (!stashFindPointer(vrmls_by_trick_file, trick_file_name, &vl))
    {
        vl = malloc(sizeof(VrmlList));
        vl->vrmls = 0;
        eaCreate(&vl->vrmls);
        stashAddPointer(vrmls_by_trick_file, trick_file_name, vl, false);
    }
    else
    {
        int i, len = eaSize(&vl->vrmls);
        for (i = 0; i < len; i++)
        {
            if (stricmp(vl->vrmls[i], vrml_name)==0)
                return;
        }
    }

    eaPush(&vl->vrmls, strdup(vrml_name));
}

static void addVrmlToLODHash(char *vrml_name, char *geo_name)
{
    VrmlList *vl;
    char lod_file_name[MAX_PATH];

    if (!vrmls_by_lod_file)
        vrmls_by_lod_file = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);

    if (!fileLocateWrite(getLODFileName(geo_name), lod_file_name))
        return;

    if (!stashFindPointer(vrmls_by_lod_file, lod_file_name, &vl))
    {

        vl = malloc(sizeof(VrmlList));
        vl->vrmls = 0;
        eaCreate(&vl->vrmls);
        stashAddPointer(vrmls_by_lod_file, lod_file_name, vl, false);
    }
    else
    {
        int i, len = eaSize(&vl->vrmls);
        for (i = 0; i < len; i++)
        {
            if (stricmp(vl->vrmls[i], vrml_name)==0)
                return;
        }
    }

    eaPush(&vl->vrmls, strdup(vrml_name));
}

typedef struct
{
    char    *objname;
    char    *fname;
} GroupName;

#define MAX_GROUPNAMES 100000
GroupName group_names[MAX_GROUPNAMES];

int        group_name_count;
StashTable htGroupNames=0;

int addGroupName(char *fname, char *objname, int fatal)
{
    char buf[256], *s;
    GroupName *gn;
    if (!htGroupNames) {
        htGroupNames = stashTableCreateWithStringKeys(12000, StashDeepCopyKeys);
    }
    stripGeoName(objname, buf);
    objname = buf;

    s = strstri(fname, "object_library");
    if (s)
        fname = s;

    if (strnicmp(objname,"grp",3)==0)
        FatalErrorf("map piece %s in object library!\nIn File: %s",objname,fname);

    if (stashFindPointer(htGroupNames, objname, &gn)) {
        if (stricmp(gn->fname, fname)!=0) {
            if (fatal)
                FatalErrorf("Duplicate name:\n  %s:%s\n  %s:%s\nPLEASE FIX AND RERUN!\n", gn->fname,gn->objname,fname,objname);
            else
                Errorf("Duplicate name:\n  %s:%s\n  %s:%s\nNOT processing\n", gn->fname,gn->objname,fname,objname);
            return 1;
        }
    } else {
        // Add it!
        group_names[group_name_count].fname = strdup(fname);
        group_names[group_name_count].objname = strdup(objname);
        stashAddPointer(htGroupNames, objname, &group_names[group_name_count], false);
        group_name_count++;
        assert(group_name_count < ARRAY_SIZE(group_names));
    }
    return 0;
}

//given a wrl path (as from object_library... converts it to txt in target
//only from addGroupNames
static char *wrlNameToTxtName(char *fname,char *buf)
{
char    *s,buf2[1000];

    makefullpath(fname,buf2);
    srcToData(buf2,buf);
    if (strEndsWith(buf, "(null)"))
        strcpy(buf, buf2);
    s = strrchr(buf,'.');
    strcpy(s,".rootnames");
    return buf;
}

//wacky group stuff only from object_library processing area
static void addGroupNames(char *fname)
{
FILE    *file;
char    buf[1000],fname2[1000],*s;

    wrlNameToTxtName(fname,fname2);
    //N:\game\data\object_library\city_zones\venice\venicewalls\venicewalls.txt
    // no writing done here? _chmod(fname2, _S_IWRITE | _S_IREAD);
    file = fopen(fname2,"rt");
    if (!file)
        return;
    while(fgets(buf,sizeof(buf),file))
    {
        if (strncmp(buf,"Def ",4)==0)
        {
            buf[strlen(buf)-1] = 0;
            s = buf + 4;
            addGroupName(fname, s, 1);
        }
    }
    fclose(file);
}


void checkVrmlName(char *fname)
{
    char buf[1000];
    char *vrmlname, *dirname, *s;
    Strncpyt(buf, fname);
    forwardSlashes(buf);
    vrmlname = strrchr(buf, '/');
    *(vrmlname++)=0;
    s = strrchr(vrmlname, '.');
    assert(stricmp(s, ".wrl")==0);
    *s=0;
    dirname = strrchr(buf, '/');
    assert(dirname);
    dirname++;
}

static FILE* getvrml_lock_handle=NULL;
static char *lockfilename = "c:\\getvrml.lock";
static void releaseGetvrmlLock() {
    fclose(getvrml_lock_handle);
    fileForceRemove(lockfilename);
    getvrml_lock_handle = 0;
}

static void waitForGetvrmlLock() {
    fileForceRemove(lockfilename);
    while ((getvrml_lock_handle = fopen(lockfilename, "wl")) == NULL) {
        Sleep(500);
        fileForceRemove(lockfilename);
    }
    if (getvrml_lock_handle > 0) {
        fprintf(getvrml_lock_handle, "getvrml.exe\r\n");
    }
}



typedef enum Reasons {
    REASON_PROCESSED,
    REASON_NOTYOURS,
    REASON_NOTNEWER,
    REASON_CHECKOUTFAILED,
} Reasons;

static Reasons processVrml2(char *out_fname, char *vrml_name, char *out_group_fname, int force_rebuild, int targetlibrary )
{
    int isNewer = 0;
    int lodsChanged = 0;

    if (force_rebuild || (isNewer=fileNewer(out_fname,vrml_name)) || (lodsChanged = (targetlibrary != SCALE_LIBRARY && lodParamsChanged(out_fname, vrml_name, targetlibrary == PLAYER_LIBRARY))))
    {
        int    ret;
        int is_legacy = (targetlibrary == PLAYER_LIBRARY) && isLegacy(out_fname, vrml_name);
        const char * addFiles[3] = {NULL, NULL, NULL};

        /* rules for processing:
        When GetVrml sees a .WRL file that is newer than its corresponding .geo file, it sees that this
        file needs to be processed.  Previously it would then just check out the .geo file and process it.
        Now, it will NOT process the file if the .WRL is checked out by someone else.  If no one has the
        file checked out, then it will only process it if you were the last person to check it in.  There
        should no longer be any issues with people getting their .geo files checked out by someone else.

        The correct procedure to things is :
        1. Make your changes (Check out .WRL file)
        2. Process the geometry (run GetVRML)
        3. TEST (Run the game)
        4. Check-in (or check-point) your files so other people can get them
        */

        // vrml_name == .wrl file
        // out_fname == output .geo file

        // Don't do this anymore, since we're only processing our own files, we don't care
        //if (fileNewer(out_fname, vrml_name)) { // Get the latest version just to make sure
        //    perforceAdd(out_fname, PERFORCE_PATH_FILE);
        //    ret=perforceSubmit(out_fname, PERFORCE_PATH_FILE, "AUTO: processVrml2");
        //}

        // add vrml file to source control if not there already
        addFiles[0] = vrml_name;

        printf("\n");
        //            if (!fileExists(out_fname)) {
        //                FILE *fnew;
        mkdirtree(out_fname);
        //                fnew = fopen(out_fname, "w");
        //                fclose(fnew);
        //            }

        _chmod(out_fname, _S_IWRITE | _S_IREAD);
        ret = NO_ERROR;

        if (!doReLODWarningCheck(vrml_name, out_fname, targetlibrary == PLAYER_LIBRARY))
            return REASON_NOTNEWER;

        if (lodsChanged)
            printf("LOD parameters have changed!  ");

        // remove .bak file of the .geo, it may have inadvertently been created from the step
        // above that created an empty file so that it could be checked out
        {
            char bakname[MAX_PATH];
            strcpy(bakname, out_fname);
            strcat(bakname, ".bak");
            fileForceRemove(bakname);
        }

        if(targetlibrary == OBJECT_LIBRARY)
        {
            checkVrmlName(vrml_name);
            mkdirtree(out_fname);  
            geoAddFile(vrml_name, out_fname, MERGE_NODES, PROCESS_ALL_NODES, 0, is_legacy);
            if (!foundDuplicateNames(vrml_name))
            {
                outputData(out_fname);
                outputGroupInfo(out_group_fname);
            }
        }
        else if(targetlibrary == PLAYER_LIBRARY)
        {
            geoAddFile(vrml_name, out_fname, MERGE_NODES, PROCESS_GEO_ONLY, 0, is_legacy);
            outputData(out_fname);
        }
        else if(targetlibrary == SCALE_LIBRARY)
        {
            geoAddFile(vrml_name, out_fname, NO_MERGE_NODES, PROCESS_SCALEBONES_ONLY, 0, is_legacy);
            outputData(out_fname);
        }

        // reload anim header
        {
            GeoLoadData *gld;
            geoSetExistenceErrorReporting(0);
            gld = geoLoad(out_fname, LOAD_HEADER, GEO_USED_BY_WORLD);
            geoSetExistenceErrorReporting(1);
            if (gld)
            {
                stashRemovePointer(glds_ht, gld->name, NULL);
                modelListFree(gld);
                geoLoad(out_fname, LOAD_RELOAD | LOAD_HEADER, GEO_USED_BY_WORLD);
            }
        }

        outputResetVars(); 
        printf("Done.");
        return 0;
    } else {
        return REASON_NOTNEWER;
    }
}
//hmm
static Reasons processVrml(char *fname, int force_rebuild, int targetlibrary )
{
    char    vrml_name[_MAX_PATH],*s,buf2[_MAX_PATH],name_buf[_MAX_PATH],
            out_fname[_MAX_PATH],out_group_fname[_MAX_PATH],*name;
    int ret;
    char *ext = ".geo";

    assert(targetlibrary == PLAYER_LIBRARY || targetlibrary == OBJECT_LIBRARY || targetlibrary == SCALE_LIBRARY); 

    makefullpath(fname,vrml_name); //add cwd to 
    strcpy(name_buf,vrml_name);
    name = strrchr(name_buf,'/');
    name++;
    s = strrchr(name,'.');
    if (s)
        *s = 0;
    srcToData(vrml_name,buf2);
    s = strrchr(buf2,'/');
    s[1] = 0;

    if(targetlibrary == OBJECT_LIBRARY)
    {
        sprintf(out_fname,"%s%s%s",buf2,name,ext);
        sprintf(out_group_fname,"%s%s.rootnames",buf2,name);
    }
    else if(targetlibrary == PLAYER_LIBRARY)
    {
        sprintf(out_fname,"%s%s%s%s",buf2, "player_library\\", name, ext);
    }
    else if(targetlibrary == SCALE_LIBRARY)
    {
        sprintf(out_fname,"%s%s%s%s",buf2, "player_library\\", name, ext);
    }

    // Clear the line
    printf("\r%-200c\r", ' ');
    printf("%s",vrml_name);

    ret = processVrml2(out_fname, vrml_name, out_group_fname, force_rebuild, targetlibrary);

    // add vrml name to hash table for all trick files its models use
    {
        int j;

        GeoLoadData *gld;
        geoSetExistenceErrorReporting(0);
        gld = geoLoad(out_fname, LOAD_HEADER, GEO_USED_BY_WORLD);
        geoSetExistenceErrorReporting(1);
        if (gld)
        {
            for (j=0; j<gld->modelheader.model_count; j++)
            {
                Model *model = gld->modelheader.models[j];
                char *trick_file_name = 0;
                TrickInfo *trick = 0;
                int lod_num = getAutoLodNum(model->name);
                if (lod_num > 0)
                    continue;
                trick = trickFromObjectName(model->name, model->filename);
                if (trick)
                    trick_file_name = trick->file_name;

                addVrmlToTrickHash(fname, out_fname, trick_file_name?trick_file_name:"");
            }
            addVrmlToLODHash(fname, out_fname);
        }
    }

    return ret;
}

static GMeshReductions *unpackReductions(Model *model)
{
    int deltalen;
    U8 *ptr, *reduction_data;
    void *new_data;
    GMeshReductions *reductions;

    reductions = calloc(sizeof(GMeshReductions), 1);

    ptr = reduction_data = malloc(model->pack.reductions.unpacksize);
    geoUnpack(&model->pack.reductions, reduction_data, model->name, model->filename);

#define UNPACK_INT                *((int *)ptr); ptr += sizeof(int)
#define UNPACK_INTS(count)        new_data = malloc(count * sizeof(int)); memcpy(new_data, ptr, count * sizeof(int)); ptr += count * sizeof(int)
#define UNPACK_FLOATS(count)    new_data = malloc(count * sizeof(float)); memcpy(new_data, ptr, count * sizeof(float)); ptr += count * sizeof(float)

    reductions->num_reductions = UNPACK_INT;
    reductions->num_tris_left = UNPACK_INTS(reductions->num_reductions);
    reductions->error_values = UNPACK_FLOATS(reductions->num_reductions);
    reductions->remaps_counts = UNPACK_INTS(reductions->num_reductions);
    reductions->changes_counts = UNPACK_INTS(reductions->num_reductions);

    reductions->total_remaps = UNPACK_INT;
    reductions->remaps = UNPACK_INTS(reductions->total_remaps * 3);
    reductions->total_remap_tris = UNPACK_INT;
    reductions->remap_tris = UNPACK_INTS(reductions->total_remap_tris);

    reductions->total_changes = UNPACK_INT;
    reductions->changes = UNPACK_INTS(reductions->total_changes);

    deltalen = UNPACK_INT;
    reductions->positions = malloc(reductions->total_changes * sizeof(Vec3));
    uncompressDeltas(reductions->positions, ptr, 3, reductions->total_changes, PACK_F32);
    ptr += deltalen;

    deltalen = UNPACK_INT;
    reductions->tex1s = malloc(reductions->total_changes * sizeof(Vec2));
    uncompressDeltas(reductions->tex1s, ptr, 2, reductions->total_changes, PACK_F32);
    ptr += deltalen;

    assert(ptr == reduction_data + model->pack.reductions.unpacksize);

    free(reduction_data);

    return reductions;

#undef UNPACK_INT
#undef UNPACK_INTS
#undef UNPACK_FLOATS
}


static void fillNodeFromModel(Node* node, Model *model)
{
    int i;
    U8 * weights;
    U8 * matidxs;
    int *tris;
    int j, k, usage = 0;

    node->next = node->child = node->parent = node->prev = node->nodeptr = NULL;
    for (i=0; i<3; i++) {
        node->scale[i] = model->scale[i];
    }
    strcpy(node->name, model->name);
    node->anim_id = model->id;
    if (model->api) {
        node->altMatCount = model->api->altpivotcount;
        for(i = 0; i < model->api->altpivotcount ; i++)
        {
            copyMat4(model->api->altpivot[i], node->altMat[i]);
            node->altMatUsed[i] = 1;
        }
    }

    // Unkown whether this is right/used in output:
    copyMat4(unitmat, node->mat); // This fine?
    setVec3(node->translate, 0, 0, 0);
    setVec3(node->pivot, 0, 0, 0);
    setVec3(node->rotate, 0, 0, 0); node->rotate[3] = 0;
    setVec3(node->scaleOrient, 0, 0, 0); node->scaleOrient[3] = 0;
    setVec3(node->center, 0, 0, 0);
    // Assuming these are not outputted in GetVrml:
    //AnimKeys    poskeys;
    //AnimKeys    rotkeys;

    // Fill in Shape
    //Shape        shape;

    node->radius = model->radius;
    copyVec3(model->min, node->min);
    copyVec3(model->max, node->max);
    //node->lightmap_size = model->lightmap_size;

    ZeroStruct(&node->mesh);
    if (model->pack.verts.unpacksize)
        usage |= USE_POSITIONS;
    if (model->pack.norms.unpacksize)
        usage |= USE_NORMALS;
    if (model->pack.sts.unpacksize)
        usage |= USE_TEX1S;
    if (model->pack.weights.unpacksize)
        usage |= USE_BONEWEIGHTS;
    gmeshSetUsageBits(&node->mesh, usage);
    gmeshSetVertCount(&node->mesh, model->vert_count);

    if (model->pack.verts.unpacksize)
        geoUnpackDeltas(&model->pack.verts,node->mesh.positions,3,model->vert_count,PACK_F32,model->name,model->filename);
    if (model->pack.norms.unpacksize)
        geoUnpackDeltas(&model->pack.norms,node->mesh.normals,3,model->vert_count,PACK_F32,model->name,model->filename);
    if (model->pack.sts.unpacksize)
        geoUnpackDeltas(&model->pack.sts,node->mesh.tex1s,2,model->vert_count,PACK_F32,model->name,model->filename);
    if (model->pack.weights.unpacksize)
    {
        weights = _alloca(model->vert_count*1);
        matidxs = _alloca(model->vert_count*2);
        geoUnpack(&model->pack.weights,weights,model->name,model->filename);
        geoUnpack(&model->pack.matidxs,matidxs,model->name,model->filename);
        for(i=0;i<model->vert_count;i++)
        {
            node->mesh.boneweights[i][0] = weights[i] * 1/255.f;
            node->mesh.boneweights[i][1] = 1.f - node->mesh.boneweights[i][0];
            node->mesh.bonemats[i][0] = matidxs[i*2+0];
            node->mesh.bonemats[i][1] = matidxs[i*2+1];
        }
    }
    if (model->pack.reductions.unpacksize)
    {
        node->reductions = unpackReductions(model);
    }

    // Fill in Tris
    gmeshSetTriCount(&node->mesh, model->tri_count);
    tris = _alloca(model->tri_count * 3 * sizeof(int));
    geoUnpackDeltas(&model->pack.tris,tris,3,model->tri_count,PACK_U32,model->name,model->filename);

    j=0;
    for (i=0; i<model->tex_count; i++)
    {
        TexID *texid = &model->tex_idx[i];
        k = texid->count;
        while (k)
        {
            int ii;
            node->mesh.tris[j].tex_id = texid->id;
            for (ii=0; ii<3; ii++)
                node->mesh.tris[j].idx[ii] = tris[j*3+ii];
            k--;
            j++;
        }
    }
    assert(j == model->tri_count);

    assert(!model->boneinfo); // TODO: Implement this?
    // Have not yet loaded these parameters in BoneData::
    //
    //    int            numbones;
    //    int            bone_ID[MAX_OBJBONES];
    //    int            num_bonesections;
    //    BoneSection    bonesections[MAX_BONE_SECTIONS];


    // Grid (recreate)
    //    node->shape.grid = model->grid;
    //    node->shape.grid.cell = calloc(model->pack.grid.unpacksize, 1);
    //    geoUnpack(&model->pack.grid, node->shape.grid.cell);
    //    node->shape.grid.cell = polyCellUnpack(model,node->shape.grid.cell,(void*)node->shape.grid.cell);
    gmeshUpdateGrid(&node->mesh, 0);

}

static void fillGlobalsFromGeo(GeoLoadData * gld)
{
    int i;
    texNameClear(0);
    for (i=0; i<gld->texnames.count; i++) {
        texNameAdd(gld->texnames.strings[i]);
    }
}

// The command-line helper is declared below with the rest of GetVrml's
// argument helpers.  Keep the declaration here because the offline geometry
// authoring commands intentionally live next to their serialization code.
static int checkForArg(int argc, char **argv, char * str);

static void atlasWriteJsonString(FILE *file, const char *value)
{
    const unsigned char *cursor = (const unsigned char *)(value ? value : "");

    fputc('"', file);
    for (; *cursor; cursor++)
    {
        switch (*cursor)
        {
            case '\\': fputc('\\', file); fputc('\\', file); break;
            case '"':  fputc('\\', file); fputc('"', file); break;
            case '\n': fputc('\\', file); fputc('n', file); break;
            case '\r': fputc('\\', file); fputc('r', file); break;
            case '\t': fputc('\\', file); fputc('t', file); break;
            default:   fputc(*cursor, file); break;
        }
    }
    fputc('"', file);
}

static U32 atlasHashBytes(U32 hash, const void *data, size_t size)
{
    const U8 *bytes = (const U8 *)data;
    size_t i;

    for (i = 0; i < size; i++)
    {
        hash ^= bytes[i];
        hash *= 16777619U;
    }
    return hash;
}

static const char *atlasTextureName(const GeoLoadData *gld, int tex_id)
{
    if (tex_id >= 0 && tex_id < gld->texnames.count && gld->texnames.strings[tex_id])
        return gld->texnames.strings[tex_id];
    return "white";
}

static void atlasWriteVec3(FILE *file, const Vec3 value, int mirror_x)
{
    fprintf(file, "          %.9g %.9g %.9g,\n",
        mirror_x ? -value[0] : value[0], value[1], value[2]);
}

static void atlasWriteVec2(FILE *file, const Vec2 value)
{
    fprintf(file, "          %.9g %.9g,\n", value[0], value[1]);
}

static int atlasWriteGeoVrml(const GeoLoadData *gld, const char *wrl_path)
{
    FILE *file;
    int model_index;

    file = fopen(wrl_path, "wt");
    if (!file)
    {
        printf("Unable to write extracted VRML %s\n", wrl_path);
        return 0;
    }

    fprintf(file,
        "#VRML V2.0 utf8\n"
        "# COH_ATLAS_GEO2WRL v1\n"
        "# Packed model names, arrays, triangle winding and material runs are emitted verbatim.\n\n");

    for (model_index = 0; model_index < gld->modelheader.model_count; model_index++)
    {
        const Model *model = gld->modelheader.models[model_index];
        U32 *tris = 0;
        Vec3 *positions = 0;
        Vec3 *normals = 0;
        Vec2 *texcoords = 0;
        int run_index;
        int tri_start = 0;

        if (model->tri_count)
            tris = calloc(model->tri_count * 3, sizeof(*tris));
        if (model->vert_count)
        {
            positions = calloc(model->vert_count, sizeof(*positions));
            normals = calloc(model->vert_count, sizeof(*normals));
            texcoords = calloc(model->vert_count, sizeof(*texcoords));
        }

        if (model->tri_count)
            modelGetTris(tris, (Model *)model);
        if (model->vert_count)
        {
            modelGetVerts(positions, (Model *)model);
            if (model->pack.norms.unpacksize)
                modelGetNorms(normals, (Model *)model);
            if (model->pack.sts.unpacksize)
                modelGetSts(texcoords, (Model *)model);
        }

        fprintf(file,
            "DEF %s Transform {\n"
            "  translation 0 0 0\n"
            "  scale %.9g %.9g %.9g\n"
            "  children [\n",
            model->name,
            model->scale[0], model->scale[1], model->scale[2]);

        for (run_index = 0; run_index < model->tex_count; run_index++)
        {
            const TexID *run = &model->tex_idx[run_index];
            int tri_count = run->count;
            int tri_index;

            if (tri_count <= 0)
                continue;

            fprintf(file,
                "    Shape {\n"
                "      appearance Appearance {\n"
                "        material Material { diffuseColor 1 1 1 }\n"
                "        texture ImageTexture { url ");
            atlasWriteJsonString(file, atlasTextureName(gld, run->id));
            fprintf(file,
                " }\n"
                "      }\n"
                "      geometry DEF %s-FACES-%d IndexedFaceSet {\n"
                "        ccw FALSE\n"
                "        solid TRUE\n"
                "        coord DEF %s-COORD-%d Coordinate { point [\n",
                model->name, run_index, model->name, run_index);

            for (tri_index = 0; tri_index < model->vert_count; tri_index++)
                atlasWriteVec3(file, positions[tri_index], 1);
            fprintf(file, "          ] }\n");

            fprintf(file, "        normal Normal { vector [\n");
            for (tri_index = 0; tri_index < model->vert_count; tri_index++)
                atlasWriteVec3(file, normals[tri_index], 1);
            fprintf(file, "          ] }\n");

            fprintf(file,
                "        normalPerVertex TRUE\n"
                "        texCoord DEF %s-TEXCOORD-%d TextureCoordinate { point [\n",
                model->name, run_index);
            for (tri_index = 0; tri_index < model->vert_count; tri_index++)
                atlasWriteVec2(file, texcoords[tri_index]);
            fprintf(file, "          ] }\n");

            fprintf(file, "        coordIndex [\n");
            for (tri_index = 0; tri_index < tri_count; tri_index++)
            {
                int source_tri = tri_start + tri_index;
                fprintf(file, "          %u, %u, %u, -1,\n",
                    tris[source_tri * 3 + 0],
                    tris[source_tri * 3 + 1],
                    tris[source_tri * 3 + 2]);
            }
            fprintf(file, "          ]\n        texCoordIndex [\n");
            for (tri_index = 0; tri_index < tri_count; tri_index++)
            {
                int source_tri = tri_start + tri_index;
                fprintf(file, "          %u, %u, %u, -1,\n",
                    tris[source_tri * 3 + 0],
                    tris[source_tri * 3 + 1],
                    tris[source_tri * 3 + 2]);
            }
            fprintf(file, "          ]\n        normalIndex [\n");
            for (tri_index = 0; tri_index < tri_count; tri_index++)
            {
                int source_tri = tri_start + tri_index;
                fprintf(file, "          %u, %u, %u, -1,\n",
                    tris[source_tri * 3 + 0],
                    tris[source_tri * 3 + 1],
                    tris[source_tri * 3 + 2]);
            }
            fprintf(file,
                "          ]\n"
                "      }\n"
                "    }\n");

            tri_start += tri_count;
        }

        fprintf(file, "  ]\n}\n\n");
        free(tris);
        free(positions);
        free(normals);
        free(texcoords);
    }

    fclose(file);
    return 1;
}

static void atlasWriteJsonVec3(FILE *file, const Vec3 value)
{
    fprintf(file, "[%.9g, %.9g, %.9g]", value[0], value[1], value[2]);
}

static int atlasWriteGeoFacts(const GeoLoadData *gld, const char *facts_path)
{
    FILE *file;
    int model_index;

    file = fopen(facts_path, "wt");
    if (!file)
    {
        printf("Unable to write geometry facts %s\n", facts_path);
        return 0;
    }

    fprintf(file, "{\n  \"schema\": \"coh.atlas-geo-facts.v1\",\n  \"headerName\": ");
    atlasWriteJsonString(file, gld->modelheader.name);
    fprintf(file, ",\n  \"modelCount\": %d,\n  \"textures\": [", gld->modelheader.model_count);
    for (model_index = 0; model_index < gld->texnames.count; model_index++)
    {
        if (model_index)
            fprintf(file, ", ");
        atlasWriteJsonString(file, gld->texnames.strings[model_index]);
    }
    fprintf(file, "],\n  \"models\": [\n");

    for (model_index = 0; model_index < gld->modelheader.model_count; model_index++)
    {
        const Model *model = gld->modelheader.models[model_index];
        U32 *tris = 0;
        Vec3 *positions = 0;
        Vec3 *normals = 0;
        Vec2 *texcoords = 0;
        Vec3 computed_min = { 1e30f, 1e30f, 1e30f };
        Vec3 computed_max = { -1e30f, -1e30f, -1e30f };
        F32 uv_min[2] = { 1e30f, 1e30f };
        F32 uv_max[2] = { -1e30f, -1e30f };
        F32 normal_min = 1e30f;
        F32 normal_max = 0.0f;
        int zero_normals = 0;
        int i;
        U32 position_hash = 2166136261U;
        U32 normal_hash = 2166136261U;
        U32 uv_hash = 2166136261U;
        U32 tri_hash = 2166136261U;

        if (model->tri_count)
            tris = calloc(model->tri_count * 3, sizeof(*tris));
        if (model->vert_count)
        {
            positions = calloc(model->vert_count, sizeof(*positions));
            normals = calloc(model->vert_count, sizeof(*normals));
            texcoords = calloc(model->vert_count, sizeof(*texcoords));
        }
        if (model->tri_count)
        {
            modelGetTris(tris, (Model *)model);
            tri_hash = atlasHashBytes(tri_hash, tris, model->tri_count * 3 * sizeof(*tris));
        }
        if (model->vert_count)
        {
            modelGetVerts(positions, (Model *)model);
            position_hash = atlasHashBytes(position_hash, positions, model->vert_count * sizeof(*positions));
            if (model->pack.norms.unpacksize)
            {
                modelGetNorms(normals, (Model *)model);
                normal_hash = atlasHashBytes(normal_hash, normals, model->vert_count * sizeof(*normals));
            }
            if (model->pack.sts.unpacksize)
            {
                modelGetSts(texcoords, (Model *)model);
                uv_hash = atlasHashBytes(uv_hash, texcoords, model->vert_count * sizeof(*texcoords));
            }
        }

        for (i = 0; i < model->vert_count; i++)
        {
            F32 normal_length = lengthVec3(normals[i]);
            int axis;

            for (axis = 0; axis < 3; axis++)
            {
                if (positions[i][axis] < computed_min[axis])
                    computed_min[axis] = positions[i][axis];
                if (positions[i][axis] > computed_max[axis])
                    computed_max[axis] = positions[i][axis];
            }
            if (texcoords[i][0] < uv_min[0]) uv_min[0] = texcoords[i][0];
            if (texcoords[i][1] < uv_min[1]) uv_min[1] = texcoords[i][1];
            if (texcoords[i][0] > uv_max[0]) uv_max[0] = texcoords[i][0];
            if (texcoords[i][1] > uv_max[1]) uv_max[1] = texcoords[i][1];
            if (normal_length < 0.00001f)
                zero_normals++;
            if (normal_length < normal_min) normal_min = normal_length;
            if (normal_length > normal_max) normal_max = normal_length;
        }

        if (!model->vert_count)
        {
            setVec3(computed_min, 0, 0, 0);
            setVec3(computed_max, 0, 0, 0);
            uv_min[0] = uv_min[1] = uv_max[0] = uv_max[1] = 0.0f;
            normal_min = normal_max = 0.0f;
        }

        if (model_index)
            fprintf(file, ",\n");
        fprintf(file, "    {\n      \"name\": ");
        atlasWriteJsonString(file, model->name);
        fprintf(file, ",\n      \"filename\": ");
        atlasWriteJsonString(file, model->filename);
        fprintf(file,
            ",\n      \"verts\": %d,\n      \"tris\": %d,\n      \"texCount\": %d,\n"
            "      \"radius\": %.9g,\n      \"boundsMin\": ",
            model->vert_count, model->tri_count, model->tex_count, model->radius);
        atlasWriteJsonVec3(file, model->min);
        fprintf(file, ",\n      \"boundsMax\": ");
        atlasWriteJsonVec3(file, model->max);
        fprintf(file, ",\n      \"computedMin\": ");
        atlasWriteJsonVec3(file, computed_min);
        fprintf(file, ",\n      \"computedMax\": ");
        atlasWriteJsonVec3(file, computed_max);
        fprintf(file,
            ",\n      \"uvMin\": [%.9g, %.9g],\n      \"uvMax\": [%.9g, %.9g],\n"
            "      \"normalLengthMin\": %.9g,\n      \"normalLengthMax\": %.9g,\n"
            "      \"zeroNormals\": %d,\n      \"positionHash\": \"%08X\",\n"
            "      \"normalHash\": \"%08X\",\n      \"uvHash\": \"%08X\",\n"
            "      \"triangleHash\": \"%08X\",\n      \"materials\": [",
            uv_min[0], uv_min[1], uv_max[0], uv_max[1],
            normal_min, normal_max, zero_normals,
            position_hash, normal_hash, uv_hash, tri_hash);

        {
            int run_index;
            for (run_index = 0; run_index < model->tex_count; run_index++)
            {
                if (run_index)
                    fprintf(file, ", ");
                fprintf(file, "{\"name\": ");
                atlasWriteJsonString(file, atlasTextureName(gld, model->tex_idx[run_index].id));
                fprintf(file, ", \"triangles\": %d}", model->tex_idx[run_index].count);
            }
        }
        fprintf(file,
            "],\n      \"metadata\": {\"flags\": %u, \"scale\": ",
            model->flags);
        atlasWriteJsonVec3(file, model->scale);
        fprintf(file,
            ", \"pack\": {\"tris\": %u, \"verts\": %u, \"norms\": %u, "
            "\"sts\": %u, \"sts3\": %u, \"weights\": %u, \"matidxs\": %u, "
            "\"grid\": %u, \"reductions\": %u, \"reflectionQuads\": %u}}\n    }",
            model->pack.tris.unpacksize,
            model->pack.verts.unpacksize,
            model->pack.norms.unpacksize,
            model->pack.sts.unpacksize,
            model->pack.sts3.unpacksize,
            model->pack.weights.unpacksize,
            model->pack.matidxs.unpacksize,
            model->pack.grid.unpacksize,
            model->pack.reductions.unpacksize,
            model->pack.reflection_quads.unpacksize);

        free(tris);
        free(positions);
        free(normals);
        free(texcoords);
    }

    fprintf(file, "\n  ]\n}\n");
    fclose(file);
    return 1;
}

static int atlasRunGeoTool(int argc, char **argv, int geo_index)
{
    const char *geo_path;
    const char *wrl_path = 0;
    const char *facts_path = 0;
    GeoLoadData *gld;
    int arg_index;
    int result = 0;

    if (geo_index <= 0 || geo_index >= argc)
    {
        printf("-geo2wrl/-geofacts requires a source .geo path.\n");
        return 2;
    }
    geo_path = argv[geo_index];

    arg_index = checkForArg(argc, argv, "-wrlout");
    if (arg_index > 0 && arg_index < argc)
        wrl_path = argv[arg_index];
    arg_index = checkForArg(argc, argv, "-factsout");
    if (arg_index > 0 && arg_index < argc)
        facts_path = argv[arg_index];

    if (!wrl_path && !facts_path)
    {
        printf("-geo2wrl/-geofacts requires -wrlout and/or -factsout.\n");
        return 2;
    }

    initBackgroundLoader();
    printf("Atlas geo tool: loading %s\n", geo_path);
    // This path needs packed position/normal/UV/index arrays.  The historical
    // GEO_GETVRML_FASTLOAD flag intentionally turns LOAD_NOW into header-only
    // loading for the legacy info printer, so it must not be used here.
    gld = geoLoad(geo_path, LOAD_NOW, GEO_USED_BY_WORLD);
    printf("Atlas geo tool: loaded=%p\n", (void *)gld);
    if (!gld)
    {
        printf("Unable to load source geometry %s\n", geo_path);
        return 3;
    }

    if (wrl_path)
    {
        result |= !atlasWriteGeoVrml(gld, wrl_path);
    }
    if (facts_path)
    {
        result |= !atlasWriteGeoFacts(gld, facts_path);
    }

    printf("Atlas geo tool: source=%s models=%d wrl=%s facts=%s\n",
        geo_path, gld->modelheader.model_count,
        wrl_path ? wrl_path : "(none)", facts_path ? facts_path : "(none)");
    return result ? 4 : 0;
}

static int geoHasSts3(GeoLoadData *gld)
{
    int i;
    for (i = 0; i < gld->modelheader.model_count; i++)
    {
        Model *model = gld->modelheader.models[i];
        if (!model->pack.sts3.unpacksize)
            return 0;
    }

    return 1;
}

//look for an argument in command line params
static int checkForArg(int argc, char **argv, char * str)
{
    int i;
    for(i = 0 ; i < argc ; i++)
    {
        if (!strcmp(argv[i], str))
            return i+1;
    }
    return 0;
}


static char    **names=0;

char **fileGetFlatListOfAllFilesInADirectoryRecur(char *dir,int *count_ptr)
{
struct _finddata_t fileinfo;
int        handle,test;
char    buf[1200];

    fileLocateWrite(dir, buf);
    strcat(buf,"/*");

    for(test = handle = _findfirst(buf,&fileinfo);test >= 0;test = _findnext(handle,&fileinfo))
    {
        if (fileinfo.name[0] == '.' || fileinfo.name[0] == '_')
            continue;
        sprintf(buf,"%s/%s",dir,fileinfo.name);
        names = realloc(names,(*count_ptr+1) * sizeof(names[0]));
        names[*count_ptr] = malloc(strlen(buf)+1);
        strcpy(names[*count_ptr],buf);
        (*count_ptr)++;

        if (fileinfo.attrib & _A_SUBDIR)
        {
            fileGetFlatListOfAllFilesInADirectoryRecur(buf,count_ptr);
        }
        
    }
    _findclose(handle);
    return names;
}

/*given a directory, it returns an string array of the full path to each of the files in that directory,
and it fills count_ptr with the number of files found.  files and sub-folders prefixed with "_" or "." 
are ignored.  Note: Jonathan's fileLocate is used, so if dir is absolute "C:\..", it's used unchanged, but 
if dir is relative, it will use the gamedir thing.  
*/
char **fileGetFlatListOfAllFilesInADirectory(char *dir,int *count_ptr)
{
    names=0;
    *count_ptr = 0;
    return fileGetFlatListOfAllFilesInADirectoryRecur(dir,count_ptr);
}

static char basePath[MAX_PATH];
static int targetlibrary;

static void reprocessVrml(char *fullpath, int print_errors)
{
    static char lastpath[MAX_PATH];
    static U32 lasttime=0;
    Reasons reason;

    {
        // Form the (real) full path here, otherwise fileWaitForExclusiveAccess calls 
        //    fileLocateWrite which assumes c:\game\data, but wrl files are in c:\game\src
        char vrml_name[_MAX_PATH];
        makefullpath(fullpath,vrml_name); //add cwd to 
        fileWaitForExclusiveAccess(vrml_name);
    }

    waitForGetvrmlLock();
    reason = processVrml(fullpath, g_force_rebuild, targetlibrary);
    releaseGetvrmlLock();

    if (reason == REASON_NOTNEWER && stricmp(lastpath, fullpath)==0 && timerCpuSeconds() - lasttime  < 20) {
        // print nothing
    } else if (reason != REASON_PROCESSED && print_errors) {
        printf("\nDidn't reprocess '%s' because either it isn't newer, it's not yours, or an error occurred.\n", fullpath);
        if (reason == REASON_NOTNEWER) {
            printf("  Reason:  File is not newer\n");
        }
        if (reason == REASON_NOTYOURS) {
            printf("  Reason:  Perforce thinks it's not yours\n");
        }
        if (reason == REASON_CHECKOUTFAILED) {
            printf("  Reason:  Checkout failed\n");
        }
    }

    strcpy(lastpath, fullpath);
    lasttime = timerCpuSeconds();

    printf("\r%-200c\r", ' ');
}

static void reprocessOnTheFly(const char *relpath, int when)
{
    char fullpath[MAX_PATH];

    printf("\n");

    if (strstr(relpath, "/_"))
    {
        printf("File '%s' begins with an underscore, not processing.\n", relpath);
        return;
    }

    sprintf(fullpath, "%s%s", basePath, relpath);
    reprocessVrml(fullpath, 1);
    printf("\n");
}

static void trickReloaded(const char *relpath)
{
    VrmlList *vl;
    int i, len;

    if (relpath[0] == '/')
        relpath++;

    if (!stashFindPointer(vrmls_by_trick_file, relpath, &vl))
        return;

    len = eaSize(&vl->vrmls);
    for (i = 0; i < len; i++)
    {
        reprocessVrml(vl->vrmls[i], 0);
    }
}

static void lodReloaded(const char *relpath)
{
    VrmlList *vl;
    int i, len;
    char fullpath[MAX_PATH];

    if (relpath[0] == '/')
        relpath++;

    if (!fileLocateWrite(relpath, fullpath) || !stashFindPointer(vrmls_by_lod_file, fullpath, &vl))
        return;

    len = eaSize(&vl->vrmls);
    for (i = 0; i < len; i++)
    {
        reprocessVrml(vl->vrmls[i], 0);
    }
}

void getVrmlMonitor(char *folder, int _targetlibrary)
{
    FolderCache *fcvrmllib = FolderCacheCreate();
    char path[MAX_PATH];

    targetlibrary = _targetlibrary;

    strcpy(path, folder);
    forwardSlashes(path);
    if (!strEndsWith(path, "/"))
        strcat(path, "/");
    strcpy(basePath,path);
    loadstart_printf("\nCaching %s...", path);
    FolderCacheAddFolder(fcvrmllib, path, 0);
    FolderCacheQuery(fcvrmllib, NULL);
    loadend_printf("");

    consoleSetFGColor(COLOR_GREEN | COLOR_BRIGHT);
    printf("Now monitoring for changes in .WRL files.\n");
    consoleSetDefaultColor();

    trickSetReloadCallback(trickReloaded);
    lodinfoSetReloadCallback(lodReloaded);
    FolderCacheSetCallback(FOLDER_CACHE_CALLBACK_UPDATE, "*.wrl", reprocessOnTheFly);
    while (true) {
        FolderCacheDoCallbacks();
        checkTrickReload();
        checkLODInfoReload();
        Sleep(200);
        if (_kbhit() && _getch()=='t') {
            trickReload();
        }
    }
}

static void errorPrint(char* errMsg)
{
    printf("\n%s\n\n", errMsg);
}

void checkDataDirOutputDirConsistency(const char *argv1)
{
    char *s;
    char data_dir[MAX_PATH];
    char src_dir[MAX_PATH];
    strcpy(data_dir, fileDataDir());
    fileLocateWrite(argv1, src_dir);
    forwardSlashes(data_dir);
    forwardSlashes(src_dir);
    s = strchr(data_dir, '/'); // c:
    if (s && strchr(s+1, '/'))
        s = strchr(s+1, '/'); // c:/game
    if (s)
        *s = 0;
    s = strchr(src_dir, '/'); // c:
    if (s && strchr(s+1, '/'))
        s = strchr(s+1, '/'); // c:/game
    if (s)
        *s = 0;
    if (stricmp(data_dir, src_dir)!=0) {
        FatalErrorf("Trying to process from %s into %s, this is not allowed!", src_dir, data_dir);
    }
}

int main(int argc,char **argv)
{
    EXCEPTION_HANDLER_BEGIN
    int        force_rebuild=0, i;
    int        monitor=0;
    char    folderpath[256];
    char    cwd[256];
    char ** geonames = NULL;
    int     geocnt = 0;
    char    *fname;
    int        start_at=0, end_at=-1;
    int        do_half=0;
    bool    bFoundOne;

    // Stuff to change console size
    HANDLE    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInfo;
    SMALL_RECT    rect;
    DWORD result;
    HANDLE hMutex;

    memCheckInit();

    sharedMemorySetMode(SMM_DISABLED);
    setAssertMode(ASSERTMODE_DEBUGBUTTONS);
    FolderCacheSetManualCallbackMode(1);
    setNearSameVec3Tolerance(0.0005); // Default (0.03) is way too high!

    consoleInit(220, 500, 0);
    GetConsoleScreenBufferInfo(console_handle, &consoleScreenBufferInfo);
    //coord = GetLargestConsoleWindowSize(console_handle);
    rect = consoleScreenBufferInfo.srWindow;
    rect.Right=rect.Right>120?rect.Right:120;
    SetConsoleWindowInfo(console_handle, TRUE, &rect);
    printf("%s\n", getExecutableName());
    printf("Command line: ");
    for(i=1; i<argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    winRegisterMe("Open", ".geo");
    winRegisterMe("ObjInfo", ".geo");

    if (argc < 2)
    {
        printf("   Note: Command line options come AFTER file/folder to be processed.\n");
        printf("    'getvrml <geometry file.vrml>' = single vrml to object library\n");
        printf("    'getvrml <geometry folder>' = all vrml in folder to object library\n");
        printf("    'getvrml <player geometry folder> -g' = all vrml in folder to player library\n");
        printf("    'getvrml <anims/male.geo>' = all vrml in cwd to this .geo file  \n");
        printf("    'getvrml -geo2wrl <source.geo> -wrlout <target.wrl>' = deterministic editable VRML export\n");
        printf("    'getvrml -geofacts <source.geo> -factsout <target.json>' = deterministic geometry facts export\n");
        printf("   Command line options:\n");
        printf("\t-f = force rebuild\n");
        printf("\t-nocheckout = don't check anything out. used for testing changes to getvrml\n");
        printf("\t-noperforce = same as nocheckout\n");
        printf("\t-monitor = stay active and monitor directory for file changes\n");
        printf("\t-nopig = set folder cache mode to file system only\n");
        printf("\t-onlypig = read from pigg files ONLY\n");
        printf("\t-nolod = don't create lods\n");
        printf("\t-noscan = don't scan all files at startup\n");
        printf("\t-nomeshmend = don't run meshmender (fixes invalid tangent space by duplicating vertices, on by default)\n");
        printf("\t-atlasoffline = process a single filesystem VRML without checkout or directory scanning\n");
        printf("\t-outputdir <folder to write output> = force output to a directory\n");
        printf("\t-filelist <file_list.txt> = list of wrl files to force process (can't mix player and object library)\n");
        printf("\t-g = process player geometry (all output goes to single directory instead of keeping source parent folder hierarchy)\n");
        getch();
        exit(0);
    }

    if(checkForArg(argc, argv, "-nocheckout") || checkForArg(argc, argv, "-noperforce"))
        { no_check_out = 1; no_scan = 1; } // force no_scan otherwise reprocesses every file since cant check owner
    if(checkForArg(argc, argv, "-atlasoffline"))
        { no_check_out = 1; no_scan = 0; atlas_offline = atlas_roundtrip = 1; }
    if(checkForArg(argc, argv, "-monitor"))
        monitor = 1;
    if(checkForArg(argc, argv, "-noscan"))
        no_scan = 1;
    if(checkForArg(argc, argv, "-half1"))
        do_half = 1;
    if(checkForArg(argc, argv, "-half2"))
        do_half = 2;
    if(checkForArg(argc, argv, "-f") || checkForArg(argc, argv, "-force"))  // accept -force or -f, since gettex uses -force
        g_force_rebuild = force_rebuild = 1;
    if (checkForArg(argc, argv, "-nopig"))
        FolderCacheSetMode(FOLDER_CACHE_MODE_FILESYSTEM_ONLY);
    else if(checkForArg(argc, argv, "-onlypig"))
        FolderCacheSetMode(FOLDER_CACHE_MODE_PIGS_ONLY);
    else
        FolderCacheSetMode(FOLDER_CACHE_MODE_DEVELOPMENT_DYNAMIC);
    if (checkForArg(argc, argv, "-nolod"))
        no_lods = 1;
    if (checkForArg(argc, argv, "-nomeshmend"))
        do_meshMend = 0;
    if (checkForArg(argc, argv, "-outputdir")) {
        int index = checkForArg(argc, argv, "-outputdir");
        strcpy(g_output_dir, argv[index]);
        forwardSlashes(g_output_dir);
        if (strEndsWith(g_output_dir, "/"))
            g_output_dir[strlen(g_output_dir)-1]=0;
    }
    if (checkForArg(argc, argv, "-nogui"))
        setGuiDisable(true);

    fileAutoDataDir(no_check_out == 0);

    ErrorfSetCallback(errorPrint);

    // These bounded offline commands must run before the legacy .geo info
    // scan, which otherwise treats the source argument as an inspection-only
    // request.  fileAutoDataDir() has already selected the requested pig/file
    // cache mode before the geometry is loaded.
    {
        int geo2wrl_index = checkForArg(argc, argv, "-geo2wrl");
        int geofacts_index = checkForArg(argc, argv, "-geofacts");
        if (geo2wrl_index || geofacts_index)
        {
            int geo_index = geo2wrl_index ? geo2wrl_index : geofacts_index;
            return atlasRunGeoTool(argc, argv, geo_index);
        }
    }

    bFoundOne = false;
    for (i=0; i<argc; i++) {
        char fullpath[MAX_PATH];

        if (!strEndsWith(argv[i], ".geo"))
            continue;

        if(fileIsAbsolutePath(argv[i]))
            strcpy(fullpath, argv[i]);
        else
            sprintf(fullpath, "%s\\%s", fileDataDir(), argv[i]);
        modelPrintFileInfo(fullpath, GEO_USED_BY_WORLD|GEO_GETVRML_FASTLOAD);
        bFoundOne = true;
    }
    if(bFoundOne) {
        // under normal operation, pause for user to see results
        if(!isGuiDisabled())
            _getch();
        exit(0);
    }

    initBackgroundLoader();//mm

    setConsoleTitle("GetVrml");
    trickLoad();
    lodinfoLoad();

    hMutex = CreateMutex(NULL, 0, "Global\\CrypticGetVrml");
    result = WaitForSingleObject(hMutex, 0);
    if (!(result == WAIT_OBJECT_0 || result == WAIT_ABANDONED))
    {
        // mutex locked
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        printf("\nGetVrml is already running.\nPress any key to exit.\n");
        getch();
        exit(0);
    }

    // optional list of files to process
    if (checkForArg(argc, argv, "-filelist")) {
        char fileListPath[MAX_PATH];
        int index = checkForArg(argc, argv, "-filelist");
        strcpy(fileListPath, argv[index]);
        if( !strEndsWith(fileListPath, ".txt") ) {
            printf("\nERROR: File list \'%s\" is not a txt file.\nPress any key to exit.\n", fileListPath);
            getch();
            exit(0);
        }

        // alloc more than enough names, and read wrl files from text listing
        geonames = (char**)malloc(MAX_FNAMES * sizeof(char*));
        printf("Processing file list \"%s\"...", fileListPath);
        geocnt = getFileNames_fromList(fileListPath, geonames, ".wrl");
        printf("found %d wrl files\n", geocnt);

        monitor = 0;
        force_rebuild = 1;
    }

    // Filesystem-only authoring is also used for the deliberate loose
    // round-trip staging area, which is allowed to live outside the active
    // data directory.  Keep the legacy same-root guard for normal pig-backed
    // processing.
    if( !geonames && !checkForArg(argc, argv, "-nopig") )
        checkDataDirOutputDirConsistency(argv[1]);

    texLoadAll();
    treeInit();

    printf("\n");
    
    _getcwd(cwd, 128);

    gmeshWarnIfFullyDegenerate(1); // warn user if encounter any meshes that contain only degenerate tris

    /*2.  If "-g" is an argument, update player geometry like this:
        1. find all .wrl files in the cwd+arg[1] and subfolders
        2. if force rebuild, process every .wrl file found, if not, only process newer .wrl files
        3. process only geometry in .wrl files with "GEO_" suffixes
        4. output the results of each .wrl to "data/player_library" flat    
    */
    if(checkForArg(argc, argv, "-g"))
    {
        int had_dups=0;
        StashTable htDupNamesCheck = stashTableCreateWithStringKeys(256, StashDeepCopyKeys);

        if( !geonames )
        {
            if (argv[1][1]==':') {
                strcpy(folderpath, argv[1]);
            } else {
                // relative path
                sprintf(folderpath, "%s\\%s", cwd, argv[1]);
            }

            // allow a single file to be passed in, same as we do for object_library meshes
            if( strstri(folderpath,".wrl") && !strstri(folderpath, ".wrl.") )
            {
                force_rebuild = 1;
                geocnt = 1;
                geonames = (char**)malloc(sizeof(char*));
                geonames[0] = strdup(folderpath);
            }
            else if(!no_scan)
            {
                geonames = fileScanDir(folderpath,&geocnt); //Get a list of all the files in the folder heirarchy
            }
        }

        if (do_half==1) {
            end_at = geocnt/2;
        } else if (do_half==2) {
            start_at = geocnt/2;
        }

        waitForGetvrmlLock();
        for(i = start_at ; i < geocnt ; i++)
        {
            if (i==end_at)
                break;
            if(strstri(geonames[i], ".wrl") && !strstri(geonames[i], ".wrl."))
            {
                if (stashFindPointer(htDupNamesCheck, getFileName(geonames[i]), NULL)) {
                    printf("\rDuplicate player geometry file named %s in                                   \n      %s\n      %s\n", getFileName(geonames[i]), geonames[i], stashFindPointerReturnPointer(htDupNamesCheck, getFileName(geonames[i])));
                    had_dups = 1;
                } else {
                    stashAddPointer(htDupNamesCheck, getFileName(geonames[i]), geonames[i], false);
                }
                printf("\nProcessing file %d of %d\n", i+1, geocnt);
                processVrml(geonames[i], force_rebuild, PLAYER_LIBRARY); 
            }
        }
        releaseGetvrmlLock();

        if (had_dups) {
            printf("\r                                                                                       \n");
            printf("Duplicate player library file names detected, currently probably using one at random, or\n  whichever was processed last, please delete the ones that are not needed.\n");
            printf("Press any key to continue...                                                             \n");
            _getch();
        }
        if (monitor)
            getVrmlMonitor(folderpath, PLAYER_LIBRARY);
    }
    /*Export everything in this folder to player_library for use by the bone scaling system.  
    */
    else if(checkForArg(argc, argv, "-scale"))
    {
        char ** filenames;
        int    filecount;

        filenames = fileGetFlatListOfAllFilesInADirectory(cwd,&filecount); //Get a list of all the files in the folder heirarchy

        waitForGetvrmlLock();
        for(i = 0 ; i < filecount ; i++)
        {
            if( strstri(filenames[i], ".wrl") && !strstri(filenames[i], ".wrl.") )
            {
                processVrml(filenames[i], 1, SCALE_LIBRARY); 
            }
        }
        releaseGetvrmlLock();

        if (monitor)
            getVrmlMonitor(cwd, SCALE_LIBRARY);
    }
    /*3. Otherwise, this is geometry for the object library, so do all the crazy grouping stuff  
      1. If a .wrl file is given, process that .wrl file only. 
      2. If a folder name is given, process everything in it
    */
    else
    {
        fname = argv[1];
        forwardSlashes(fname);

        if( !geonames )
        {
            if (strstri(fname,".wrl") && !strstri(fname, ".wrl.")) //does this even work now?
            {
                force_rebuild = 1;
                geonames = (char**)malloc(sizeof(char*));
                geonames[0] = strdup(fname);
                geocnt = 1;
                monitor = 0;
            }
            else
            {    
                geonames = (char**)malloc(MAX_FNAMES * sizeof(char*)); // pre-allocate since calls are recursive
                getFileNames(fname, geonames, &geocnt, ".wrl");
            }
        }

        if (do_half==1) {
            end_at = geocnt/2;
        } else if (do_half==2) {
            start_at = geocnt/2;
        }
        
        if(!no_scan)
        {
            if (!atlas_offline)
                waitForGetvrmlLock();
            for(i=start_at;i<geocnt;i++)
            {
                if (i==end_at)
                    break;
                printf("\nProcessing file %d of %d\n", i+1, geocnt);
                processVrml(geonames[i],force_rebuild, OBJECT_LIBRARY);
            }
            if (!atlas_offline)
                releaseGetvrmlLock();
        }

        printf("\r%-200c\r", ' ');
        printf("\nCompiling Group Names and checking for duplicates...");
        for(i=0;i<geocnt;i++)
        {        
            addGroupNames(geonames[i]);
        }

        printf("done (%d total)\n\n", group_name_count);
        if (monitor)
            getVrmlMonitor(fname, OBJECT_LIBRARY);

    }

    exit(0);
    EXCEPTION_HANDLER_END
}


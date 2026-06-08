#include <utilitieslib/utils/wininclude.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <utilitieslib/utils/mathutil.h>
#include <utilitieslib/utils/file.h>
#include <utilitieslib/utils/fileutil.h>
#include <utilitieslib/utils/error.h>
#include <utilitieslib/utils/utils.h>
#include "seq/animtrack.h"
#include <utilitieslib/components/StashTable.h>
//#include "seq/animtrackanimate.h"
#include <utilitieslib/utils/strings_opt.h> //    For faster stricmp calls
#include <utilitieslib/assert/assert.h>
#include "cmdparse/cmdcommon.h"
#include <utilitieslib/components/SharedHeap.h>
#include <utilitieslib/utils/timing.h>

AnimationEngine animEngine = {0};

typedef struct BoneAnimTrackOnDisk
{
    U32    rot_idx;
    U32    pos_idx;
    U16    rot_fullkeycount;
    U16    pos_fullkeycount;
    U16    rot_count;
    U16    pos_count;
    char    id;
    char    flags;
    U16    pack_pad;
} BoneAnimTrackOnDisk;

#pragma pack(push, 1)
typedef struct SkeletonAnimTrackOnDisk
{
    int        headerSize;
    char    name[MAX_ANIM_FILE_NAME_LEN];
    char    baseAnimName[MAX_ANIM_FILE_NAME_LEN];
    F32        max_hip_displacement;
    F32        length;
    U32        bone_tracks;
    int        bone_track_count;
    int        rotation_compression_type;
    int        position_compression_type;
    U32        skeletonHeirarchy;
    U32        backupAnimTrack;
    int        loadstate;
    F32        lasttimeused;
    int        fileAge;
    int        spare_room[9];
} SkeletonAnimTrackOnDisk;
#pragma pack(pop)

STATIC_ASSERT(sizeof(BoneAnimTrackOnDisk) == 20);
STATIC_ASSERT(sizeof(BoneLink) == 12);
STATIC_ASSERT(sizeof(SkeletonAnimTrackOnDisk) == 596);

static bool animDiskRangeOk(U32 offset, size_t bytes, int file_size)
{
    return file_size >= 0 && offset <= (U32)file_size && bytes <= (size_t)file_size - offset;
}

static bool animValidateDiskHeirarchyBranch(const SkeletonHeirarchy *heirarchy, bool visited[BONES_ON_DISK], int link_count, int idx)
{
    while (idx != BONEID_INVALID)
    {
        const BoneLink *link;

        if (idx < 0 || idx >= link_count || visited[idx])
            return false;

        link = &heirarchy->skeleton_heirarchy[idx];
        if (link->id != idx || !bone_IdIsValid(link->id))
            return false;
        if (link->child != BONEID_INVALID && (link->child < 0 || link->child >= link_count))
            return false;
        if (link->next != BONEID_INVALID && (link->next < 0 || link->next >= link_count))
            return false;

        visited[idx] = true;
        if (link->child != BONEID_INVALID && !animValidateDiskHeirarchyBranch(heirarchy, visited, link_count, link->child))
            return false;

        idx = link->next;
    }

    return true;
}

static bool animValidateDiskHeirarchy(const SkeletonHeirarchy *heirarchy, size_t link_count)
{
    bool visited[BONES_ON_DISK] = {0};

    if (!link_count || link_count > BONES_ON_DISK)
        return false;

    return animValidateDiskHeirarchyBranch(heirarchy, visited, (int)link_count, heirarchy->heirarchy_root);
}

static bool animCopyDiskHeirarchy(SkeletonHeirarchy *dst, const U8 *src, size_t bytes)
{
    const BoneLink *links;
    size_t link_count;
    int i;

    if (bytes < sizeof(S32))
        return false;

    memcpy(&dst->heirarchy_root, src, sizeof(dst->heirarchy_root));
    links = (const BoneLink*)(src + sizeof(S32));
    bytes -= sizeof(S32);

    if (bytes % sizeof(BoneLink) != 0)
        return false;

    link_count = bytes / sizeof(BoneLink);
    if (!link_count || link_count > BONES_ON_DISK)
        return false;

    for (i = 0; i < BONES_ON_DISK; i++)
    {
        dst->skeleton_heirarchy[i].child = BONEID_INVALID;
        dst->skeleton_heirarchy[i].next = BONEID_INVALID;
        dst->skeleton_heirarchy[i].id = BONEID_INVALID;
    }

    for (i = 0; i < (int)link_count; i++)
    {
        dst->skeleton_heirarchy[i].child = links[i].child;
        dst->skeleton_heirarchy[i].next = links[i].next;
        dst->skeleton_heirarchy[i].id = (BoneId)links[i].id;
    }

    if (animValidateDiskHeirarchy(dst, link_count))
        return true;

    /*
     * Some asset files carry a shortened hierarchy block. Keep the load alive
     * by dropping links outside the copied span without inventing asset-specific
     * exceptions.
     */
    if (dst->heirarchy_root < 0 || dst->heirarchy_root >= (int)link_count)
        return false;

    for (i = 0; i < (int)link_count; i++)
    {
        BoneLink *link = &dst->skeleton_heirarchy[i];

        if (link->id != i || !bone_IdIsValid(link->id))
        {
            link->child = BONEID_INVALID;
            link->next = BONEID_INVALID;
            link->id = BONEID_INVALID;
            continue;
        }

        if (link->child != BONEID_INVALID && (link->child < 0 || link->child >= (int)link_count))
            link->child = BONEID_INVALID;
        if (link->next != BONEID_INVALID && (link->next < 0 || link->next >= (int)link_count))
            link->next = BONEID_INVALID;
    }

    return dst->skeleton_heirarchy[dst->heirarchy_root].id == dst->heirarchy_root;
}

static void animDebugCheckBoneTrackOnLoad( BoneAnimTrack * bt, const char *fileName )
{
    int i;

    /////////////// Debug ///////////////////////////////////////////////////
    if(
        bt->flags & ROTATION_COMPRESSED_TO_5_BYTES
        || bt->flags & ROTATION_COMPRESSED_NONLINEAR
        ) //TEMP DEBUG
    {
        for(i = 0 ; i < bt->rot_fullkeycount ; i++ )
        {
            if ( ((U8*)bt->rot_idx)[ i * 5 ] >= 64 )
                FatalErrorFilenamef(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
        }
    }
    else if(bt->flags & ROTATION_UNCOMPRESSED ) //TEMP DEBUG
    {
        F32 * quat;
        for(i = 0 ; i < bt->rot_fullkeycount ; i++ )
        {
            quat = (F32*)&((F32*)bt->rot_idx)[ i * 4 ];
            if (!finiteVec4( quat ))
                FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
            if ( !quat[ 0 ] && !quat[ 1 ] && !quat[ 2 ] && !quat[ 3 ] )
                FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
        }
    }
    if( bt->flags & POSITION_UNCOMPRESSED ) //TEMP DEBUG 
    {
        F32 * pos;
        for( i = 0 ; i < bt->pos_fullkeycount ; i++ )
        {
              pos = (F32*)&(((F32*)bt->pos_idx)[i*3]);
            if (!finiteVec3( pos ))
                FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
            if (!( pos[0] < POS_BIGGEST && pos[1] < POS_BIGGEST && pos[2] < POS_BIGGEST ))
                FatalErrorf(fileName, "Bad animation data loaded.  Your data might be corrupt, please run the patcher and try again.");
        }
    }
}

static SkeletonAnimTrack *animConvertTrackFile(const U8 *file_data, int file_size, bool load_all)
{
    const SkeletonAnimTrackOnDisk *disk = (const SkeletonAnimTrackOnDisk*)file_data;
    const BoneAnimTrackOnDisk *disk_bones;
    SkeletonAnimTrack *skeleton;
    BoneAnimTrack *bone_tracks;
    SkeletonHeirarchy *heirarchy;
    U8 *raw_data;
    size_t alloc_size;
    size_t heirarchy_size;
    size_t disk_heirarchy_size;
    int i;

    if (!file_data)
    {
        return NULL;
    }
    if (file_size < (int)sizeof(*disk))
    {
        return NULL;
    }
    if (disk->bone_track_count < 0)
    {
        return NULL;
    }
    if (disk->bone_track_count > BONES_ON_DISK)
    {
        return NULL;
    }
    if (!animDiskRangeOk(disk->bone_tracks, (size_t)disk->bone_track_count * sizeof(BoneAnimTrackOnDisk), file_size))
    {
        return NULL;
    }
    if (!disk->skeletonHeirarchy)
        disk_heirarchy_size = 0;
    else if (disk->bone_tracks > disk->skeletonHeirarchy)
        disk_heirarchy_size = disk->bone_tracks - disk->skeletonHeirarchy;
    else
        disk_heirarchy_size = (U32)file_size - disk->skeletonHeirarchy;

    if (disk->skeletonHeirarchy &&
        !animDiskRangeOk(disk->skeletonHeirarchy, disk_heirarchy_size, file_size))
    {
        return NULL;
    }

    disk_bones = (const BoneAnimTrackOnDisk*)(file_data + disk->bone_tracks);
    if (load_all)
    {
        for (i = 0; i < disk->bone_track_count; i++)
        {
            if (disk_bones[i].rot_idx && disk_bones[i].rot_idx >= (U32)file_size)
            {
                return NULL;
            }
            if (disk_bones[i].pos_idx && disk_bones[i].pos_idx >= (U32)file_size)
            {
                return NULL;
            }
        }
    }

    heirarchy_size = disk->skeletonHeirarchy ? sizeof(SkeletonHeirarchy) : 0;
    alloc_size = sizeof(SkeletonAnimTrack) + disk->bone_track_count * sizeof(BoneAnimTrack) + heirarchy_size + file_size;
    skeleton = calloc(1, alloc_size);
    if (!skeleton)
    {
        return NULL;
    }

    bone_tracks = (BoneAnimTrack*)((U8*)skeleton + sizeof(SkeletonAnimTrack));
    heirarchy = disk->skeletonHeirarchy ? (SkeletonHeirarchy*)(bone_tracks + disk->bone_track_count) : NULL;
    raw_data = disk->skeletonHeirarchy ? (U8*)(heirarchy + 1) : (U8*)(bone_tracks + disk->bone_track_count);
    memcpy(raw_data, file_data, file_size);
    disk_bones = (const BoneAnimTrackOnDisk*)(raw_data + disk->bone_tracks);

    skeleton->headerSize = disk->headerSize;
    memcpy(skeleton->name, disk->name, sizeof(skeleton->name));
    skeleton->name[sizeof(skeleton->name) - 1] = 0;
    memcpy(skeleton->baseAnimName, disk->baseAnimName, sizeof(skeleton->baseAnimName));
    skeleton->baseAnimName[sizeof(skeleton->baseAnimName) - 1] = 0;
    skeleton->max_hip_displacement = disk->max_hip_displacement;
    skeleton->length = disk->length;
    skeleton->bone_tracks = bone_tracks;
    skeleton->bone_track_count = disk->bone_track_count;
    skeleton->rotation_compression_type = disk->rotation_compression_type;
    skeleton->position_compression_type = disk->position_compression_type;
    if (heirarchy)
    {
        if (!animCopyDiskHeirarchy(heirarchy, raw_data + disk->skeletonHeirarchy, disk_heirarchy_size))
        {
            free(skeleton);
            return NULL;
        }
        skeleton->skeletonHeirarchy = heirarchy;
    }
    else
    {
        skeleton->skeletonHeirarchy = NULL;
    }
    skeleton->backupAnimTrack = NULL;
    skeleton->loadstate = disk->loadstate;
    skeleton->lasttimeused = disk->lasttimeused;
    skeleton->fileAge = disk->fileAge;
    memcpy(skeleton->spare_room, disk->spare_room, sizeof(skeleton->spare_room));

    for (i = 0; i < disk->bone_track_count; i++)
    {
        bone_tracks[i].rot_idx = (load_all && disk_bones[i].rot_idx) ? raw_data + disk_bones[i].rot_idx : NULL;
        bone_tracks[i].pos_idx = (load_all && disk_bones[i].pos_idx) ? raw_data + disk_bones[i].pos_idx : NULL;
        bone_tracks[i].rot_fullkeycount = disk_bones[i].rot_fullkeycount;
        bone_tracks[i].pos_fullkeycount = disk_bones[i].pos_fullkeycount;
        bone_tracks[i].rot_count = disk_bones[i].rot_count;
        bone_tracks[i].pos_count = disk_bones[i].pos_count;
        bone_tracks[i].id = disk_bones[i].id;
        bone_tracks[i].flags = disk_bones[i].flags;
        bone_tracks[i].pack_pad = disk_bones[i].pack_pad;
    }

    return skeleton;
}

static SkeletonAnimTrack * animReadTrackFile( FILE * file, int load_type, const char* fileName, SharedHeapAcquireResult* ret ) 
{
    SkeletonAnimTrack * skeleton;
    int file_size, i;
    BoneAnimTrack * bt;
    void * track_data = 0;
    SharedHeapHandle* pHandle = NULL;

#ifdef _M_X64
    if ( load_type == LOAD_ALL_SHARED )
    {
        load_type = LOAD_ALL;
        *ret = SHAR_Error;
    }
#endif

    if ( load_type == LOAD_ALL_SHARED )
    {
        *ret = sharedHeapAcquire(&pHandle, fileName);
        if ( *ret == SHAR_DataAcquired )
        {
            // the data is already there, just return the pointer from the handle
            assert( pHandle );
            return pHandle->data;
        }
        else if ( *ret == SHAR_Error )
        {
            // Just load all normally
            load_type = LOAD_ALL; 
        }
        else // SHAR_FirstCaller
        {

        }
    }
    else
    {
        // HYPNOS TODO: Looks like this function was written to support
        // both 'shared' load and unshared, but now fails for anything but
        // shared. Could simplify if that is a requirement now
        *ret = SHAR_Error;
    }


    if( load_type == LOAD_ALL || load_type == LOAD_ALL_SHARED
#ifdef _M_X64
        || load_type == LOAD_SKELETONS_ONLY
#endif
        )
    {
        file_size = fileGetSize( file );
    }
    else //LOAD_SKELETONS_ONLY
    {
        fread( &file_size, sizeof( int ), 1, file );
        fseek( file, 0, SEEK_SET );
    }

    if ( load_type == LOAD_ALL_SHARED )
    {
        bool bSuccess = sharedHeapAlloc( pHandle, file_size );

        if ( !bSuccess )
        {
            sharedHeapMemoryManagerUnlock();
            pHandle = NULL;
            load_type = LOAD_ALL;
            *ret = SHAR_Error;
            skeleton = malloc(file_size);
        }
        else
        {
            skeleton = pHandle->data;
        }
    }
    else 
        skeleton = malloc( file_size );

    assert( skeleton );
    fread( skeleton, 1, file_size, file );

#ifdef _M_X64
    {
        bool load_all = (load_type == LOAD_ALL);
        SkeletonAnimTrack *converted = animConvertTrackFile((U8*)skeleton, file_size, load_all);
        free(skeleton);
        skeleton = converted;
        if (!skeleton)
            return NULL;

        if (load_all)
        {
            for( i = 0 ; i < skeleton->bone_track_count ; i++ )
                animDebugCheckBoneTrackOnLoad( &skeleton->bone_tracks[i], fileName );
        }

        return skeleton;
    }
#endif

    skeleton->backupAnimTrack = NULL;

    skeleton->bone_tracks = (void*)((size_t)skeleton->bone_tracks + (size_t)skeleton);

    if( load_type == LOAD_ALL || load_type == LOAD_ALL_SHARED ) //Fix up the data ptrs.
    {
        for( i = 0 ; i < skeleton->bone_track_count ; i++ )
        {
            bt = &(skeleton->bone_tracks[i]);

            bt->pos_idx = (void*)((size_t)bt->pos_idx + (size_t)skeleton);
            bt->rot_idx = (void*)((size_t)bt->rot_idx + (size_t)skeleton);

            // HYPNOS TODO: This is slowing down streaming and should
            // be done in a batch sweep and at tool conversion time
            animDebugCheckBoneTrackOnLoad( bt, fileName );
        }
    }

    if( skeleton->skeletonHeirarchy )
        skeleton->skeletonHeirarchy = (void*)((size_t)skeleton->skeletonHeirarchy + (size_t)skeleton);

    if ( load_type == LOAD_ALL_SHARED )
        sharedHeapMemoryManagerUnlock();
    return skeleton;
}

// fixup pointers and patch data post load
void animPatchLoadedRawData( SkeletonAnimTrack * skeleton )
{
    int i;

    skeleton->backupAnimTrack = NULL;
    skeleton->bone_tracks = (void*)((UPTR)skeleton->bone_tracks + (UPTR)skeleton);

    for( i = 0 ; i < skeleton->bone_track_count ; i++ )
    {
        BoneAnimTrack* bt = &(skeleton->bone_tracks[i]);

        bt->pos_idx = (void*)((UPTR)bt->pos_idx + (UPTR)skeleton);
        bt->rot_idx = (void*)((UPTR)bt->rot_idx + (UPTR)skeleton);
    }

    if( skeleton->skeletonHeirarchy )
        skeleton->skeletonHeirarchy = (void*)((UPTR)skeleton->skeletonHeirarchy + (UPTR)skeleton);
}

// walk the track data looking for anomalies
void animValidateTrackData( SkeletonAnimTrack * skeleton )
{
    int i;

    for( i = 0 ; i < skeleton->bone_track_count ; i++ )
    {
        BoneAnimTrack* bt = &(skeleton->bone_tracks[i]);
        animDebugCheckBoneTrackOnLoad( bt, skeleton->name );
    }
}

SkeletonAnimTrack * animLoadAnimFile( const char* fileName )
{
    SkeletonAnimTrack* pTrack = NULL;
    void *file_data = NULL;
    int file_size;
    FILE* f;

    f = fopen(fileName, "rb");
    if (!f)
    {
        Errorf("Could not load anim file");
        return NULL;
    }

    file_size = fileGetSize( f );
    file_data = malloc(file_size);
    if (!file_data)
    {
        Errorf("Not enough memory to load anim file");
        fclose(f);
        return NULL;
    }
    fread( file_data, 1, file_size, f );

#ifdef _M_X64
    pTrack = animConvertTrackFile(file_data, file_size, true);
    free(file_data);
    if (!pTrack)
    {
        fclose(f);
        return NULL;
    }
#else
    pTrack = file_data;
    animPatchLoadedRawData( pTrack );
#endif
    animValidateTrackData( pTrack );

    fclose(f);

    return pTrack;
}

static void UpCase(char *s)
{
    for(;*s;s++) *s = toupper(*s);
}

static SkeletonAnimTrack* animDevCheckUpdated(SkeletonAnimTrack* animTrack, const char* fileName, const char* nameClean)
{
    if(!global_state.no_file_change_check && isDevelopmentMode())
    {  
        if( fileHasBeenUpdated( fileName, &animTrack->fileAge ) ) //can change fileAge
        {
            stashRemovePointer( animEngine.tracksLoadedHT, nameClean , &animTrack);
            animTrack = 0; //Don't bother freeing the old one, someone might be using it
            animEngine.tracksLoadedCount--;
            assert( animEngine.tracksLoadedCount >= 0 );
        }
    }
    
    return animTrack;
}

static void animDevSetFileAge(SkeletonAnimTrack* animTrack, const char* fileName)
{
    if(!global_state.no_file_change_check && isDevelopmentMode())
    {
        animTrack->fileAge = 0;
        fileHasBeenUpdated( fileName, &animTrack->fileAge );
    }
}

static void animLinkToBaseSkeleton(SkeletonAnimTrack* animTrack, int loadType, const char* fileName, int useSharedMemory)
{
    SkeletonAnimTrack * backupAnimTrack;

    if ( useSharedMemory )
        sharedHeapMemoryManagerUnlock();
    backupAnimTrack = animGetAnimTrack( animTrack->baseAnimName, loadType, fileName );
    if ( useSharedMemory )
        sharedHeapMemoryManagerLock();
    animTrack->backupAnimTrack = backupAnimTrack;
    animTrack->skeletonHeirarchy = backupAnimTrack->skeletonHeirarchy;

    assert( backupAnimTrack && backupAnimTrack->skeletonHeirarchy );

    animTrack->max_hip_displacement = 4.0; //temp for testing visibility
}


SkeletonAnimTrack * animGetAnimTrack( const char * name, int loadType, const char *requestor )
{
    StashElement element;
    SkeletonAnimTrack * animTrack;
    char nameClean[MAX_ANIM_FILE_NAME_LEN];
    char fileName[MAX_ANIM_FILE_NAME_LEN];
    bool bUsingSharedMemory = (loadType == LOAD_ALL_SHARED);

    if( !name || !name[0] )
    {
        ErrorFilenamef(requestor, "ANIM: animGetAnimTrack called with no name\n" );
        return 0;
    }
    
    PERFINFO_AUTO_START("animGetAnimTrack", 1);

    PERFINFO_AUTO_START("top", 1);
    
    assert( strlen(name) < MAX_ANIM_FILE_NAME_LEN );
    strcpy( nameClean, name );
    UpCase(nameClean);

    //nameClean = something like "MALE/SKEL_READY" 
    //sprintf( fileName, "player_library/animations/%s.anim", nameClean );
    STR_COMBINE_BEGIN(fileName);
    STR_COMBINE_CAT("player_library/animations/");
    STR_COMBINE_CAT(nameClean);
    STR_COMBINE_CAT(".anim");
    STR_COMBINE_END();

    if( !animEngine.tracksLoadedHT )
    {
        animEngine.tracksLoadedHT = stashTableCreateWithStringKeys(10000, StashDeepCopyKeys);
    }
    
    animTrack = 0;

    /*Look in the hash table for this already loaded.  If so, return that.*/
    stashFindElement( animEngine.tracksLoadedHT, nameClean , &element);
    if( element )
    {
        animTrack = (SkeletonAnimTrack*)stashElementGetPointer(element);

        if(animTrack && !bUsingSharedMemory)
        {
            //Development-Only animation reloader.
            
            animTrack = animDevCheckUpdated(animTrack, fileName, nameClean);
        }
    }
    
    PERFINFO_AUTO_STOP_START("middle", 1);

    //If you haven't loaded it yet, or it's been updated, then load it up.
    if(!animTrack)
    {
        FILE * file = 0;
        bool bShouldWriteToSharedMemory = false;
        SharedHeapAcquireResult ret = SHAR_Error;

        PERFINFO_AUTO_STOP_START("middle2", 1);

        //TO DO the half opening thing for svr
        file = fileOpen( fileName, "rb");
        if( !file )
        {
            ErrorFilenamef(requestor, "ANIM: animation file '%s' doesn't exist!\n", fileName);
            PERFINFO_AUTO_STOP();
            PERFINFO_AUTO_STOP();
            return 0;
        }

        animTrack = animReadTrackFile( file, loadType, fileName, &ret );

        fileClose( file );

        if ( ret == SHAR_Error )
        {
            bUsingSharedMemory = false;
            bShouldWriteToSharedMemory = false;
        }
        else if ( ret == SHAR_DataAcquired )
        {
            bUsingSharedMemory = true;
            bShouldWriteToSharedMemory = false;
        }
        else // ret == SHAR_FirstCaller
        {
            bUsingSharedMemory = true;
            bShouldWriteToSharedMemory = true;
        }
        
        PERFINFO_AUTO_STOP_START("middle3", 1);

        //########### add it to the list of loaded stuff and check dev
        //Do before animGetAnimTrack below so you don't get caught in loop 
        stashAddPointer( animEngine.tracksLoadedHT, nameClean, animTrack, false);
        animEngine.tracksLoadedCount++;

        if ( bUsingSharedMemory && !bShouldWriteToSharedMemory && !animTrack->backupAnimTrack)
        {
            // We should always have a backupanimtrack... something is wrong, so write to shared memory to fix it
            bShouldWriteToSharedMemory = true;
        }

        if ( bShouldWriteToSharedMemory )
        {
            PERFINFO_AUTO_STOP_START("middle4", 1);

            // Lock the heap memory manager, so we can finish writing to this animtrack
            sharedHeapMemoryManagerLock();
        }

        // only do development mode stuff if we're not using shared memory
        //Development mode initialize fileAge
        if(!bUsingSharedMemory)
        {
            PERFINFO_AUTO_STOP_START("middle5", 1);

            animDevSetFileAge(animTrack, fileName);
        }
        //end development

        //########### A few minor initializations...
        //link to baseSkeleton
        if (!animTrack->backupAnimTrack && ( !bUsingSharedMemory || bShouldWriteToSharedMemory ) )
        {
            PERFINFO_AUTO_STOP_START("middle6", 1);

            animLinkToBaseSkeleton(animTrack, loadType, fileName, bShouldWriteToSharedMemory);
        }

        assert( animTrack->backupAnimTrack && animTrack->backupAnimTrack->skeletonHeirarchy );

        if ( bShouldWriteToSharedMemory )
        {
            PERFINFO_AUTO_STOP_START("middle7", 1);

            // Unlock the heap memory manager
            sharedHeapMemoryManagerUnlock();
        }


    }

    PERFINFO_AUTO_STOP();

    PERFINFO_AUTO_STOP();

    return animTrack;
}

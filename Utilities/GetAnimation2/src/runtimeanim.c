#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utilitieslib/stdtypes.h>
#include <utilitieslib/components/earray.h>
#include <utilitieslib/utils/file.h>
#include <utilitieslib/utils/fileutil.h>
#include <utilitieslib/utils/mathutil.h>
#include <utilitieslib/utils/quat.h>
#include <utilitieslib/utils/utils.h>

#include "processanim.h"
#include "runtimeanim.h"
#include "seq/animtrackanimate.h"
#include "seq/bones.h"

typedef struct RuntimeHierarchy
{
    int parent[BONES_ON_DISK];
    int order[BONES_ON_DISK];
    int count;
    bool visited[BONES_ON_DISK];
} RuntimeHierarchy;

static void runtimeSetIdentity(Quat q)
{
    q[0] = 0.0f;
    q[1] = 0.0f;
    q[2] = 0.0f;
    q[3] = 1.0f;
}

static const BoneAnimTrack *runtimeFindTrack(const SkeletonAnimTrack *anim, int id, bool *fromBase)
{
    const BoneAnimTrack *track = NULL;

    if (fromBase)
        *fromBase = false;

    if (!anim)
        return NULL;

    track = animFindTrackInSkeleton(anim, id);
    if (!track && anim->backupAnimTrack)
    {
        track = animFindTrackInSkeleton(anim->backupAnimTrack, id);
        if (track && fromBase)
            *fromBase = true;
    }

    return track;
}

static bool runtimeGetLocal(const SkeletonAnimTrack *anim, int id, int frame,
                            Quat rotation, Vec3 translation, bool *fromBase)
{
    const BoneAnimTrack *track = runtimeFindTrack(anim, id, fromBase);

    if (!track)
    {
        runtimeSetIdentity(rotation);
        zeroVec3(translation);
        return false;
    }

    animGetRotationValue(frame, 0.0f, track, rotation, 0);
    animGetPositionValue(frame, 0.0f, track, translation, 0);
    return finiteVec4(rotation) && finiteVec3(translation);
}

static int runtimeCountChildren(const SkeletonHeirarchy *hierarchy, int id)
{
    int child;
    int count = 0;

    if (!hierarchy || id < 0 || id >= BONES_ON_DISK)
        return 0;

    child = hierarchy->skeleton_heirarchy[id].child;
    while (child >= 0 && child < BONES_ON_DISK)
    {
        ++count;
        child = hierarchy->skeleton_heirarchy[child].next;
    }
    return count;
}

static void runtimeCollectSiblings(const SkeletonHeirarchy *hierarchy, int id,
                                   int parent, RuntimeHierarchy *result)
{
    while (hierarchy && id >= 0 && id < BONES_ON_DISK)
    {
        const BoneLink *link = &hierarchy->skeleton_heirarchy[id];

        if (result->visited[id])
            return;
        result->visited[id] = true;
        result->parent[id] = parent;
        if (result->count < BONES_ON_DISK)
            result->order[result->count++] = id;

        runtimeCollectSiblings(hierarchy, link->child, id, result);
        id = link->next;
    }
}

static bool runtimeBuildHierarchy(const SkeletonAnimTrack *anim, RuntimeHierarchy *result)
{
    int i;

    memset(result, 0, sizeof(*result));
    for (i = 0; i < BONES_ON_DISK; ++i)
        result->parent[i] = -1;

    if (!anim || !anim->skeletonHeirarchy)
        return false;

    runtimeCollectSiblings(anim->skeletonHeirarchy,
                           anim->skeletonHeirarchy->heirarchy_root,
                           -1, result);
    return result->count > 0;
}

static int runtimeMaxFrame(const SkeletonAnimTrack *anim)
{
    int maxFrame = (int)(anim && anim->length > 0.0f ? anim->length + 0.5f : 0.0f);
    const SkeletonAnimTrack *tracks[2];
    int trackSetCount = 0;
    int setIndex;
    int i;

    if (!anim)
        return 0;

    tracks[trackSetCount++] = anim;
    if (anim->backupAnimTrack)
        tracks[trackSetCount++] = anim->backupAnimTrack;

    for (setIndex = 0; setIndex < trackSetCount; ++setIndex)
    {
        const SkeletonAnimTrack *trackSet = tracks[setIndex];
        for (i = 0; i < trackSet->bone_track_count; ++i)
        {
            const BoneAnimTrack *track = &trackSet->bone_tracks[i];
            int trackFrame = max((int)track->rot_count, (int)track->pos_count) - 1;
            if (trackFrame > maxFrame)
                maxFrame = trackFrame;
        }
    }

    return maxFrame;
}

static void runtimeGameQuatToSource(const Quat gameRotation, Vec3 sourceAxis, F32 *sourceAngle)
{
    Quat normalized;
    Vec3 gameAxis;

    memcpy(normalized, gameRotation, sizeof(Quat));
    quatNormalize(normalized);
    quatToAxisAngle(normalized, gameAxis, sourceAngle);

    /*
     * This is the inverse of the exact conversion used by process_animx.c:
     * source axis -> ConvertCoordsFrom3DSMAX(axis), source angle -> -angle.
     * Negating the converted axis keeps the exported angle positive while
     * preserving the same rotation when GetAnimation2 reads it again.
     */
    ConvertCoordsGameTo3DSMAX(sourceAxis, gameAxis);
    sourceAxis[0] = -sourceAxis[0];
    sourceAxis[1] = -sourceAxis[1];
    sourceAxis[2] = -sourceAxis[2];

    if (fabs(*sourceAngle) < 1e-6f || lengthVec3Squared(sourceAxis) < 1e-10f)
    {
        sourceAxis[0] = 0.0f;
        sourceAxis[1] = 1.0f;
        sourceAxis[2] = 0.0f;
        *sourceAngle = 0.0f;
    }
}

static void runtimeBuildWorldFrame(const SkeletonAnimTrack *anim,
                                   const RuntimeHierarchy *hierarchy, int frame,
                                   Quat worldRotation[BONES_ON_DISK],
                                   Vec3 worldTranslation[BONES_ON_DISK])
{
    int i;

    for (i = 0; i < hierarchy->count; ++i)
    {
        int id = hierarchy->order[i];
        int parent = hierarchy->parent[id];
        Quat localRotation;
        Vec3 localTranslation;
        bool fromBase;

        runtimeGetLocal(anim, id, frame, localRotation, localTranslation, &fromBase);
        (void)fromBase;

        if (parent < 0)
        {
            memcpy(worldRotation[id], localRotation, sizeof(Quat));
            copyVec3(localTranslation, worldTranslation[id]);
        }
        else
        {
            Vec3 delta;

            /*
             * process_animx.c reconstructs local rotation as
             *   qLocal = qWorld * inverse(qParentWorld)
             * and local position as
             *   rotate(inverse(qParentWorld), world-parent).
             * Inverting those exact operations gives the composition below.
             */
            quatMultiply(localRotation, worldRotation[parent], worldRotation[id]);
            quatRotateVec3(worldRotation[parent], localTranslation, delta);
            addVec3(worldTranslation[parent], delta, worldTranslation[id]);
        }
    }
}

static void runtimeWriteQuat(FILE *file, const char *label, const Quat value)
{
    fprintf(file, "%s %.9g %.9g %.9g %.9g", label,
            value[0], value[1], value[2], value[3]);
}

static void runtimeWriteJsonString(FILE *file, const char *value)
{
    const unsigned char *cursor = (const unsigned char *)(value ? value : "");

    fputc('"', file);
    while (*cursor)
    {
        if (*cursor == '\\' || *cursor == '"')
            fputc('\\', file);
        fputc(*cursor, file);
        ++cursor;
    }
    fputc('"', file);
}

static void runtimeWriteJsonVec3(FILE *file, const Vec3 value)
{
    fprintf(file, "[%.9g, %.9g, %.9g]", value[0], value[1], value[2]);
}

static void runtimeWriteJsonQuat(FILE *file, const Quat value)
{
    fprintf(file, "[%.9g, %.9g, %.9g, %.9g]",
            value[0], value[1], value[2], value[3]);
}

static const char *runtimeTrackSource(const SkeletonAnimTrack *anim, int id,
                                      bool *fromBase, const BoneAnimTrack **track)
{
    const BoneAnimTrack *found = runtimeFindTrack(anim, id, fromBase);
    if (track)
        *track = found;
    if (!found)
        return "missing";
    return fromBase && *fromBase ? "base" : "animation";
}

bool runtimeAnimWriteReport(const SkeletonAnimTrack *anim, const char *path)
{
    RuntimeHierarchy hierarchy;
    FILE *file;
    int maxFrame;
    int i;
    bool frame0Reference = true;
    int positionTrackCount = 0;

    if (!runtimeBuildHierarchy(anim, &hierarchy))
    {
        printf("Runtime animation has no skeleton hierarchy: %s\n", anim ? anim->name : "<null>");
        return false;
    }

    maxFrame = runtimeMaxFrame(anim);
    mkdirtree((char *)path);
    file = fopen(path, "wt");
    if (!file)
        return false;

    for (i = 0; i < hierarchy.count; ++i)
    {
        int id = hierarchy.order[i];
        bool fromBase;
        const BoneAnimTrack *track;
        const char *source = runtimeTrackSource(anim, id, &fromBase, &track);
        if (track && track->pos_count > 0 && (!fromBase || anim->backupAnimTrack))
            ++positionTrackCount;
        {
            Quat frame0Rotation;
            Vec3 frame0Translation;
            if (!runtimeGetLocal(anim, id, 0, frame0Rotation, frame0Translation, NULL))
                frame0Reference = false;
        }
        (void)source;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"animation\": "); runtimeWriteJsonString(file, anim->name); fprintf(file, ",\n");
    fprintf(file, "  \"baseAnimName\": "); runtimeWriteJsonString(file, anim->baseAnimName); fprintf(file, ",\n");
    fprintf(file, "  \"length\": %.9g,\n", anim->length);
    fprintf(file, "  \"maxSampleFrame\": %d,\n", maxFrame);
    fprintf(file, "  \"boneCount\": %d,\n", hierarchy.count);
    fprintf(file, "  \"positionTrackCount\": %d,\n", positionTrackCount);
    fprintf(file, "  \"frame0Reference\": %s,\n", frame0Reference ? "true" : "false");
    fprintf(file, "  \"hierarchyRoot\": %d,\n", anim->skeletonHeirarchy->heirarchy_root);
    fprintf(file, "  \"bones\": [\n");

    for (i = 0; i < hierarchy.count; ++i)
    {
        int id = hierarchy.order[i];
        int parent = hierarchy.parent[id];
        const BoneLink *link = &anim->skeletonHeirarchy->skeleton_heirarchy[id];
        bool fromBase;
        const BoneAnimTrack *track;
        const char *source = runtimeTrackSource(anim, id, &fromBase, &track);
        Quat rotation;
        Vec3 translation;
        int frame;

        runtimeGetLocal(anim, id, 0, rotation, translation, &fromBase);
        fprintf(file, "    {\n");
        fprintf(file, "      \"id\": %d,\n", id);
        fprintf(file, "      \"name\": "); runtimeWriteJsonString(file, bone_NameFromId(id)); fprintf(file, ",\n");
        fprintf(file, "      \"parent\": %d,\n", parent);
        fprintf(file, "      \"child\": %d,\n", link->child);
        fprintf(file, "      \"sibling\": %d,\n", link->next);
        fprintf(file, "      \"trackSource\": \"%s\",\n", source);
        fprintf(file, "      \"rotationTrackCount\": %d,\n", track ? track->rot_count : 0);
        fprintf(file, "      \"positionTrackCount\": %d,\n", track ? track->pos_count : 0);
        fprintf(file, "      \"flags\": %d,\n", track ? track->flags : 0);
        fprintf(file, "      \"frame0LocalRotation\": "); runtimeWriteJsonQuat(file, rotation); fprintf(file, ",\n");
        fprintf(file, "      \"frame0LocalTranslation\": "); runtimeWriteJsonVec3(file, translation); fprintf(file, ",\n");
        fprintf(file, "      \"samples\": [\n");

        for (frame = 0; frame <= maxFrame; ++frame)
        {
            runtimeGetLocal(anim, id, frame, rotation, translation, NULL);
            fprintf(file, "        {\"frame\": %d, \"rotation\": ", frame);
            runtimeWriteJsonQuat(file, rotation);
            fprintf(file, ", \"translation\": ");
            runtimeWriteJsonVec3(file, translation);
            fprintf(file, "}%s\n", frame == maxFrame ? "" : ",");
        }

        fprintf(file, "      ]\n");
        fprintf(file, "    }%s\n", i + 1 == hierarchy.count ? "" : ",");
    }

    fprintf(file, "  ]\n}\n");
    fclose(file);
    return true;
}

static void runtimeWriteSKELXSiblings(FILE *file, const SkeletonAnimTrack *anim,
                                      const RuntimeHierarchy *hierarchy, int id,
                                      int depth)
{
    const SkeletonHeirarchy *skeleton = anim->skeletonHeirarchy;

    while (id >= 0 && id < BONES_ON_DISK)
    {
        Quat rotation;
        Vec3 translation;
        Vec3 axis;
        F32 angle;
        int child;
        int childCount;
        int indent;

        runtimeGetLocal(anim, id, 0, rotation, translation, NULL);
        runtimeGameQuatToSource(rotation, axis, &angle);
        ConvertCoordsGameTo3DSMAX(translation, translation);

        for (indent = 0; indent < depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "Bone \"%s\"\n", bone_NameFromId(id));
        for (indent = 0; indent < depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "{\n");
        for (indent = 0; indent <= depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "Axis %.9g %.9g %.9g\n", axis[0], axis[1], axis[2]);
        for (indent = 0; indent <= depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "Angle %.9g\n", angle);
        for (indent = 0; indent <= depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "Translation %.9g %.9g %.9g\n", translation[0], translation[1], translation[2]);
        for (indent = 0; indent <= depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "Scale 1 1 1\n");

        child = skeleton->skeleton_heirarchy[id].child;
        childCount = runtimeCountChildren(skeleton, id);
        if (childCount > 0)
        {
            for (indent = 0; indent <= depth; ++indent)
                fprintf(file, "    ");
            fprintf(file, "Children %d\n\n", childCount);
            runtimeWriteSKELXSiblings(file, anim, hierarchy, child, depth + 1);
        }

        for (indent = 0; indent < depth; ++indent)
            fprintf(file, "    ");
        fprintf(file, "}\n\n");
        id = skeleton->skeleton_heirarchy[id].next;
    }
    (void)hierarchy;
}

bool runtimeAnimWriteSKELX(const SkeletonAnimTrack *anim, const char *path)
{
    RuntimeHierarchy hierarchy;
    FILE *file;

    if (!runtimeBuildHierarchy(anim, &hierarchy))
        return false;

    mkdirtree((char *)path);
    file = fopen(path, "wt");
    if (!file)
        return false;

    fprintf(file, "# CoH runtime skeleton reconstruction\n");
    fprintf(file, "# Generated by GetAnimation2 packed-runtime inspection\n\n");
    fprintf(file, "Version 200\n");
    fprintf(file, "SourceName %s\n\n", anim->name);
    fprintf(file, "# NODE HIERARCHY\n");
    runtimeWriteSKELXSiblings(file, anim, &hierarchy,
                              anim->skeletonHeirarchy->heirarchy_root, 0);
    fclose(file);
    return true;
}

static void runtimeWriteANIMXTransform(FILE *file, const Quat rotation,
                                       const Vec3 translation)
{
    Vec3 axis;
    Vec3 sourceTranslation;
    F32 angle;

    runtimeGameQuatToSource(rotation, axis, &angle);
    ConvertCoordsGameTo3DSMAX(sourceTranslation, translation);

    fprintf(file, "        Transform\n");
    fprintf(file, "        {\n");
    fprintf(file, "            Axis %.9g %.9g %.9g\n", axis[0], axis[1], axis[2]);
    fprintf(file, "            Angle %.9g\n", angle);
    fprintf(file, "            Translation %.9g %.9g %.9g\n",
            sourceTranslation[0], sourceTranslation[1], sourceTranslation[2]);
    fprintf(file, "            Scale 1 1 1\n");
    fprintf(file, "        }\n\n");
}

bool runtimeAnimWriteANIMX(const SkeletonAnimTrack *anim, const char *path)
{
    RuntimeHierarchy hierarchy;
    FILE *file;
    int maxFrame;
    int i;
    int frame;
    Quat worldRotation[BONES_ON_DISK];
    Vec3 worldTranslation[BONES_ON_DISK];

    if (!runtimeBuildHierarchy(anim, &hierarchy))
        return false;

    maxFrame = runtimeMaxFrame(anim);
    if (maxFrame < 1)
    {
        printf("Runtime animation has no authored frames after frame 0: %s\n", anim->name);
        return false;
    }

    mkdirtree((char *)path);
    file = fopen(path, "wt");
    if (!file)
        return false;

    fprintf(file, "# CoH runtime animation round-trip export\n");
    fprintf(file, "# World/model-space transforms are converted back to the ANIMX source frame.\n\n");
    fprintf(file, "Version 200\n");
    fprintf(file, "SourceName %s\n", anim->name);
    fprintf(file, "TotalFrames %d\n", maxFrame);
    fprintf(file, "FirstFrame 0\n\n");

    for (i = 0; i < hierarchy.count; ++i)
    {
        int id = hierarchy.order[i];

        fprintf(file, "Bone \"%s\"\n", bone_NameFromId(id));
        fprintf(file, "{\n");
        for (frame = 1; frame <= maxFrame; ++frame)
        {
            runtimeBuildWorldFrame(anim, &hierarchy, frame,
                                   worldRotation, worldTranslation);
            runtimeWriteANIMXTransform(file, worldRotation[id], worldTranslation[id]);
        }
        fprintf(file, "}\n\n");
    }

    fclose(file);
    return true;
}

/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#include <utilitieslib/components/earray.h>
#include "container/containerloadsave.h"
#include "container/dbcontainerpack.h"
#include "costume_db.h"
#include "auth/authUserData.h"
#include "entity/character_level.h"
#include "entity/character_base.h"
#include "entity/entity.h"
#include "gameData/BodyPart.h"
#include "entity/powers.h"

void packageCostumes( Entity * e, StuffBuff *psb, StructDesc *desc)
{
    int i, j;
    DBCostume dbcostumes ={0};

    for( i = 0; i < e->pl->num_costumes_stored; i++ )
    {
        for( j = 0; j < MAX_COSTUME_PARTS; j++ )
        {
             if( j < e->pl->costume[i]->appearance.iNumParts )
            {

                if( (!e->pl->costume[i]->parts[j]->pchGeom || stricmp( e->pl->costume[i]->parts[j]->pchGeom, "none") == 0 ) &&
                    (!e->pl->costume[i]->parts[j]->pchFxName || stricmp( e->pl->costume[i]->parts[j]->pchFxName, "none") == 0 ) &&
                    (!e->pl->costume[i]->parts[j]->pchTex1 || stricmp( e->pl->costume[i]->parts[j]->pchTex1, "none") == 0 ) &&
                    (!e->pl->costume[i]->parts[j]->pchTex2 || stricmp( e->pl->costume[i]->parts[j]->pchTex2, "none") == 0 ) &&
                    (!e->pl->costume[i]->parts[j]->regionName || stricmp( e->pl->costume[i]->parts[j]->regionName, "none") == 0 ) &&
                    (!e->pl->costume[i]->parts[j]->bodySetName || stricmp( e->pl->costume[i]->parts[j]->bodySetName, "none") == 0 ) )
                {
                    continue;
                }

                StructCopy(ParseCostumePart, e->pl->costume[i]->parts[j],&dbcostumes.part[(i*MAX_COSTUME_PARTS)+j],0,0 );
                dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].costumeNum = i;

                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchGeom && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchGeom, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchGeom = NULL;
                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchFxName && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchFxName, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchFxName = NULL;
                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex1 && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex1, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex1 = NULL;
                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex2 && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex2, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].pchTex2 = NULL;
                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].regionName && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].regionName, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].regionName = NULL;
                if( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].bodySetName && stricmp( dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].bodySetName, "none") == 0 )
                    dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].bodySetName = NULL;

                dbcostumes.part[(i * MAX_COSTUME_PARTS) + j].costumeSlot = j;

                dbcostumes.part[(i*MAX_COSTUME_PARTS)+j].displayName = NULL;
            }
        }
    }

    for(j=0;desc->lineDescs[j].type;j++)
    {
        structToLine(psb,(char*)&dbcostumes,0,desc,&desc->lineDescs[j],0);
    }
}


void packageAppearances( Entity *e, StuffBuff *psb, StructDesc *desc )
{
    int i;
    DBAppearance dba ={0};

    for( i = 0; i < e->pl->num_costumes_stored; i++ )
    {
        compressAppearanceScales(&e->pl->costume[i]->appearance);
        memcpy( &dba.appearance[i], &e->pl->costume[i]->appearance, sizeof(Appearance) );
    }

    for(i=0;desc->lineDescs[i].type;i++)
    {
        structToLine(psb,(char*)&dba,0,desc,&desc->lineDescs[i],0);
    }
}

void packagePowerCustomizations( Entity *e, StuffBuff *psb, StructDesc *desc )
{
    int i,j;
    int currentPower = 0;
    DBPowerCustomizationList dbpc ={0};

    for( i = 0; i < e->pl->num_costumes_stored; i++ )
    {
        int numCustomizations = eaSize(&e->pl->powerCustomizationLists[i]->powerCustomizations);
        for (j = 0; j < numCustomizations; ++j)
        {
            const BasePower *power = e->pl->powerCustomizationLists[i]->powerCustomizations[j]->power;
            StructInitFields(ParseDBPowerCustomization, &dbpc.powerCustomization[currentPower]);
            if (power)
            {
                dbpc.powerCustomization[currentPower].token = e->pl->powerCustomizationLists[i]->powerCustomizations[j]->token;
                dbpc.powerCustomization[currentPower].customTint = e->pl->powerCustomizationLists[i]->powerCustomizations[j]->customTint;
                dbpc.powerCustomization[currentPower].powerCatName = power->psetParent->pcatParent->pchName;
                dbpc.powerCustomization[currentPower].powerSetName = power->psetParent->pchName;
                dbpc.powerCustomization[currentPower].powerName = power->pchName;
                dbpc.powerCustomization[currentPower].slotId = i+1;        //    0 is not set
                currentPower++;
                continue;
            }
        }
    }
    for(i=0;desc->lineDescs[i].type;i++)
    {
        structToLine(psb,(char*)&dbpc,0,desc,&desc->lineDescs[i],0);
    }
}

void unpackCostumes( Entity * e, DBCostume *dbcostumes)
{
    int i, j;
    static char none_str[] = "none";
    int numCostumes = MIN(MAX_COSTUMES,costume_get_num_slots(e));

    if (e && e->pl)
        numCostumes = MIN(MAX_COSTUMES, MAX(e->pl->num_costumes_stored, numCostumes));

    int dbMaxCostumeParts = 0;
    bool newCostume = false;

    for( i = 0; i < numCostumes; i++ )
    { 

        for( j = 0; j < MAX_COSTUME_PARTS; j++ )
        {
            int costumeNum = dbcostumes->part[(i * MAX_COSTUME_PARTS) + j].costumeNum;
            int costumeSlot = dbcostumes->part[(i * MAX_COSTUME_PARTS) + j].costumeSlot;

            // Ok, here is the logic:
            // if costumeSlot> 0, this costume was saved with new code, we just read the costume slot and num in and use those, also set a flag
            // if costumeSlot is 0 and costumeNum is 0, then we are loading slot 0 num 0 in the new code, or any slot 0 num J in the old code, so just use J
            // in either case, we can safely just use J
            // if costumeNum > 0 and costumeSlot is 0, then we are loading an old costume, and need to calculate how big MAX_COSTUME_PARTS was when we saved the DB
            // As we progress, we will either flag it as a new costume or set dbMaxCostume parts, once either of these is set, we can ignore all the below and calculate our costumeNum

            if (costumeSlot > 0) {
                newCostume = true;
            }
            else if (!newCostume && costumeNum == 0 && costumeSlot == 0) {
                // Even for old costumes, it doesn't matter what MAX_COSTUME_PARTS was for costume 0, as it's right at the start of the array
                costumeSlot = j;
            }
            else if (!newCostume && costumeNum == 1 && dbMaxCostumeParts == 0 && costumeSlot == 0) {
                //We are making an assumption here that slot 0 will *always* have something in it
                //Who knows if this will go wrong
                //Note that this will trigger on costume 1 slot 0, UNLESS, costume 0 set the newCostume flag
                dbMaxCostumeParts = (i * MAX_COSTUME_PARTS) + j;    // If MAX_COSTUME_PARTS is 60 but the costume was saved with 30, we hit costume 1 at I=0, J=30 not I=1, J=0
                assert(dbMaxCostumeParts >= MAX_COSTUME_PARTS);
                verbose_printf("Loading old costume, detected %d old MAX_COSTUME_PARTS", dbMaxCostumeParts);
            }
 
            if (costumeSlot == 0 && dbMaxCostumeParts > 0)
                costumeSlot = ((i * MAX_COSTUME_PARTS) + j) % dbMaxCostumeParts;

             if( dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].pchGeom || dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].pchFxName || 
                dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].pchTex1 || dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].pchTex2 ||
                dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].regionName || dbcostumes->part[(i*MAX_COSTUME_PARTS)+j].bodySetName)
            {    
                StructCopy(ParseCostumePart, &dbcostumes->part[(i*MAX_COSTUME_PARTS)+j],e->pl->costume[costumeNum]->parts[costumeSlot], 0,0);
            }

            if( !e->pl->costume[costumeNum]->parts[costumeSlot]->pchGeom || !e->pl->costume[costumeNum]->parts[costumeSlot]->pchGeom[0] )
                e->pl->costume[costumeNum]->parts[costumeSlot]->pchGeom = none_str;
            if( !e->pl->costume[costumeNum]->parts[costumeSlot]->pchFxName || !e->pl->costume[costumeNum]->parts[costumeSlot]->pchFxName[0] )
                e->pl->costume[costumeNum]->parts[costumeSlot]->pchFxName = none_str;
            if( !e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex1 || !e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex1[0] )
                e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex1 = none_str;
            if( !e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex2 || !e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex2[0] )
                e->pl->costume[costumeNum]->parts[costumeSlot]->pchTex2 = none_str;
            //if( !e->pl->costume[i]->parts[j]->regionName || !e->pl->costume[i]->parts[j]->regionName[0] )
            //    e->pl->costume[i]->parts[j]->regionName = none_str;
            //if(! e->pl->costume[i]->parts[j]->bodySetName || !e->pl->costume[i]->parts[j]->bodySetName[0] )
            //    e->pl->costume[i]->parts[j]->bodySetName = none_str;
        }
    }

    if( e->pl && character_IsInitialized(e->pchar) )
        costumeAwardAuthParts( e );

    // while we are at it, might as well convert supergroup to new color structure, fun fun
    if( e->supergroup && (!e->pl->superColorsPrimary || !e->pl->superColorsSecondary || !e->pl->superColorsPrimary2 || !e->pl->superColorsSecondary2   ) )
        costume_SGColorsExtract( e, e->pl->superCostume, &e->pl->superColorsPrimary, &e->pl->superColorsSecondary, &e->pl->superColorsPrimary2, &e->pl->superColorsSecondary2, &e->pl->superColorsTertiary, &e->pl->superColorsQuaternary );

}
void unpackAppearance( Entity *e, DBAppearance * dba )
{
    int i,k;
    int numCostumes = MIN(MAX_COSTUMES,costume_get_num_slots(e));

    if (e && e->pl)
        numCostumes = MIN(MAX_COSTUMES, MAX(e->pl->num_costumes_stored, numCostumes));
    // If any fields in Appearance are changed make sure that this function gets
    // updated!
#ifndef _M_X64
    STATIC_INFUNC_ASSERT(sizeof(Appearance) == 324)
#endif

    for( i = 0; i < numCostumes; i++ )
    {
        decompressAppearanceScales(&dba->appearance[i]);

        // OLD CHARACTERS FIX
        // appearance[0].bodytype -> BodyType
        // appearance[0].colorSkin -> ColorSkin
        // Don't overwrite these from the Appearances data
        if( !e->pl->costume[i] )
        {
            e->pl->costume[i] = costume_create(GetBodyPartCount());
        }

        if (i > 0)
        {
            e->pl->costume[i]->appearance.bodytype        = dba->appearance[i].bodytype;
            e->pl->costume[i]->appearance.colorSkin        = dba->appearance[i].colorSkin;
        }

        e->pl->costume[i]->appearance.convertedScale    = dba->appearance[i].convertedScale;

        for(k=0;k<MAX_BODY_SCALES;k++)
        {
            // OLD CHARACTERS FIX
            // appearance[0].fScales[0] -> BodyScale
            // appearance[0].fScales[1] -> BoneScale
            // Don't overwrite these from the Appearances data

            if (i > 0 || k > 1)
                e->pl->costume[i]->appearance.fScales[k] = dba->appearance[i].fScales[k];
        }

        for(k=0;k<NUM_SG_COLOR_SLOTS;k++)
        {
            e->pl->costume[i]->appearance.superColorsPrimary[k]    = dba->appearance[i].superColorsPrimary[k];
            e->pl->costume[i]->appearance.superColorsSecondary[k]  = dba->appearance[i].superColorsSecondary[k];
            e->pl->costume[i]->appearance.superColorsPrimary2[k]   = dba->appearance[i].superColorsPrimary2[k];
            e->pl->costume[i]->appearance.superColorsSecondary2[k] = dba->appearance[i].superColorsSecondary2[k];
            e->pl->costume[i]->appearance.superColorsTertiary[k]   = dba->appearance[i].superColorsTertiary[k];
            e->pl->costume[i]->appearance.superColorsQuaternary[k] = dba->appearance[i].superColorsQuaternary[k];
        }

        e->pl->costume[i]->appearance.currentSuperColorSet = dba->appearance[i].currentSuperColorSet;


        if(    !e->pl->costume[i]->appearance.convertedScale )
            costume_retrofitBoneScale( e->pl->costume[i] );
    }
}

#define MAX_I16_POWER_CUST_POWERS 300
static void correctCostumeIndexFromLegacyDBCust(DBPowerCustomizationList *dbpowers, int numCostumes)
{
    int currentStatus = 0;
    int i;
    for (i = 0; i < numCostumes*MAX_POWERS; ++i)
    {
        if (dbpowers->powerCustomization[i].slotId)
        {
            return;    //    we are up to date with new packing method.    early exit
        }
    }
    for (i = 1; i < numCostumes; ++i)
    {
        //    check the first power at one of these offset to try to find what state we are in
        //    if the costume has added power customization, it will always have the full complement of powers stored, regardless if they are default or not
        if (dbpowers->powerCustomization[i*MAX_I16_POWER_CUST_POWERS].powerName)
        {
            currentStatus = 0;    //    prei16
            break;
        }
        if (dbpowers->powerCustomization[i*MAX_POWERS].powerName)
        {
            currentStatus = 1;    //    posti16
            break;
        }
    }
    for (i = 0; i < numCostumes; ++i)
    {
        int j;
        int maxpowers = currentStatus ? MAX_POWERS : MAX_I16_POWER_CUST_POWERS;
        for (j = 0; j < maxpowers; ++j)
        {
            if (dbpowers->powerCustomization[(i*maxpowers)+j].powerName)
            {
                dbpowers->powerCustomization[(i*maxpowers)+j].slotId = i+1;
            }
        }
    }
}

void unpackPowerCustomizations( Entity * e, DBPowerCustomizationList *dbpowers)
{
    int i;
    int currentPowerIndex[MAX_COSTUMES];
    int numCostumes = MIN(MAX_COSTUMES,costume_get_num_slots(e));

    if (e && e->pl)
        numCostumes = MIN(MAX_COSTUMES, MAX(e->pl->num_costumes_stored, numCostumes));

    for (i = 0; i < numCostumes; ++i)
    {
        currentPowerIndex[i] = 0;
    }
    // If any fields in Appearance are changed make sure that this function gets
    // updated!
#ifndef _M_X64
    STATIC_INFUNC_ASSERT(sizeof(PowerCustomization) == 20)
#endif
    correctCostumeIndexFromLegacyDBCust(dbpowers, numCostumes);
    for( i = 0; i < numCostumes; i++ )
    {
        if (e->pl->powerCustomizationLists[i] == NULL)        e->pl->powerCustomizationLists[i] = StructAllocRaw(sizeof(PowerCustomizationList));
        powerCustList_allocPowerCustomizationList(e->pl->powerCustomizationLists[i], MAX_POWERS);
    }
    for( i = 0; i < MAX_POWERS*numCostumes; i++ )
    {
        if (&dbpowers->powerCustomization[i] && dbpowers->powerCustomization[i].powerName)
        {
            devassert(dbpowers->powerCustomization[i].slotId);    //    power didn't go into a slot. how did this happen?
            if (dbpowers->powerCustomization[i].slotId && dbpowers->powerCustomization[i].slotId <= numCostumes)
            {
                int powerIndex;
                int costumeIndex = dbpowers->powerCustomization[i].slotId-1;
                powerIndex = currentPowerIndex[costumeIndex];
                initializePowerCustomizationPower(&dbpowers->powerCustomization[i], e->pl->powerCustomizationLists[costumeIndex]->powerCustomizations[powerIndex]);
                if (!e->pl->powerCustomizationLists[costumeIndex]->powerCustomizations[powerIndex]->power)
                {
                    StructInitFields(ParsePowerCustomization, e->pl->powerCustomizationLists[costumeIndex]->powerCustomizations[powerIndex]);
                    break;
                }
                else
                {
                    e->pl->powerCustomizationLists[costumeIndex]->powerCustomizations[powerIndex]->token = dbpowers->powerCustomization[i].token;
                    e->pl->powerCustomizationLists[costumeIndex]->powerCustomizations[powerIndex]->customTint = dbpowers->powerCustomization[i].customTint;
                }
                currentPowerIndex[costumeIndex] = currentPowerIndex[costumeIndex]+1;
            }
        }
    }
    for( i = 0; i < numCostumes; i++ )
    {
        int rem;
        for (rem = (MAX_POWERS-1); rem >= currentPowerIndex[i]; --rem)
        {
            StructDestroy(ParsePowerCustomization, e->pl->powerCustomizationLists[i]->powerCustomizations[rem]);
            eaRemove(&e->pl->powerCustomizationLists[i]->powerCustomizations, rem);
        }
        eaQSort(e->pl->powerCustomizationLists[i]->powerCustomizations, comparePowerCustomizations);
    }

    powerCust_handleAllEmptyPowerCust(e);
}
/* End of File */

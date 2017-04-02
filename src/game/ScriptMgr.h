/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SCRIPTMGR_H
#define _SCRIPTMGR_H

#include "Common.h"
#include "Policies/Singleton.h"
#include "ObjectGuid.h"
#include "DBCEnums.h"
#include "ace/Atomic_Op.h"

struct AreaTriggerEntry;
class Aura;
class Creature;
class CreatureAI;
class GameObject;
class GameObjectAI;
class InstanceData;
class Item;
class Map;
class Object;
class Player;
class Quest;
class SpellCastTargets;
class Unit;
class WorldObject;

enum eScriptCommand
{
    SCRIPT_COMMAND_TALK                     = 0,            // source = WorldObject, target = any/none, datalong (see enum ChatType for supported CHAT_TYPE_'s)
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius, datalong4 = language
                                                            // data_flags = flag_target_player_as_source    = 0x01
                                                            //              flag_original_source_as_target  = 0x02
                                                            //              flag_buddy_as_target            = 0x04
                                                            // dataint = text entry from db_script_string -table. dataint2-4 optional for random selected text.
    SCRIPT_COMMAND_EMOTE                    = 1,            // source = Unit (or WorldObject when creature entry defined), target = Unit (or none)
                                                            // datalong = emote_id
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius
                                                            // data_flags = flag_target_as_source           = 0x01
    SCRIPT_COMMAND_FIELD_SET                = 2,            // source = any, datalong = field_id, datalong2 = value
    SCRIPT_COMMAND_MOVE_TO                  = 3,            // source = Creature, datalong2 = time, x/y/z
    SCRIPT_COMMAND_FLAG_SET                 = 4,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_FLAG_REMOVE              = 5,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_TELEPORT_TO              = 6,            // source or target with Player, datalong = map_id, x/y/z
    SCRIPT_COMMAND_QUEST_EXPLORED           = 7,            // one from source or target must be Player, another GO/Creature, datalong=quest_id, datalong2=distance or 0
    SCRIPT_COMMAND_KILL_CREDIT              = 8,            // source or target with Player, datalong = creature entry, datalong2 = bool (0=personal credit, 1=group credit)
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT       = 9,            // source = any (summoner), datalong=db_guid, datalong2=despawn_delay
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE     = 10,           // source = any (summoner), datalong=creature entry, datalong2=despawn_delay
    SCRIPT_COMMAND_OPEN_DOOR                = 11,           // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_CLOSE_DOOR               = 12,           // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_ACTIVATE_OBJECT          = 13,           // source = unit, target=GO
    SCRIPT_COMMAND_REMOVE_AURA              = 14,           // source (datalong2!=0) or target (datalong==0) unit, datalong = spell_id
    SCRIPT_COMMAND_CAST_SPELL               = 15,           // source/target cast spell at target/source
                                                            // datalong2: 0: s->t 1: s->s 2: t->t 3: t->s (this values in 2 bits), and 0x4 mask for cast triggered can be added to
    SCRIPT_COMMAND_PLAY_SOUND               = 16,           // source = any object, target=any/player, datalong (sound_id), datalong2 (bitmask: 0/1=anyone/target, 0/2=with distance dependent, so 1|2 = 3 is target with distance dependent)
    SCRIPT_COMMAND_CREATE_ITEM              = 17,           // source or target must be player, datalong = item entry, datalong2 = amount
    SCRIPT_COMMAND_DESPAWN_SELF             = 18,           // source or target must be creature, datalong = despawn delay
    SCRIPT_COMMAND_PLAY_MOVIE               = 19,           // target can only be a player, datalog = movie id
    SCRIPT_COMMAND_MOVEMENT                 = 20,           // source or target must be creature. datalong = MovementType (0:idle, 1:random or 2:waypoint)
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius
    SCRIPT_COMMAND_SET_ACTIVEOBJECT         = 21,           // source=any, target=creature
                                                            // datalong=bool 0=off, 1=on
                                                            // datalong2=creature entry, datalong3=search radius
    SCRIPT_COMMAND_SET_FACTION              = 22,           // source=any, target=creature
                                                            // datalong=factionId,
                                                            // datalong2=creature entry, datalong3=search radius
    SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL  = 23,           // source=any, target=creature
                                                            // datalong=creature entry/modelid (depend on data_flags)
                                                            // datalong2=creature entry, datalong3=search radius
                                                            // dataflags= 0x01 to use datalong value as modelid explicit
    SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL  = 24,           // source=any, target=creature
                                                            // datalong=creature entry/modelid (depend on data_flags)
                                                            // datalong2=creature entry, datalong3=search radius
                                                            // dataflags= 0x01 to use datalong value as modelid explicit
    SCRIPT_COMMAND_SET_RUN                  = 25,           // source=any, target=creature
                                                            // datalong= bool 0=off, 1=on
                                                            // datalong2=creature entry, datalong3=search radius
    SCRIPT_COMMAND_ATTACK_START             = 26,           // source = Creature (or WorldObject when creature entry are defined), target = Player
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius
    SCRIPT_COMMAND_GO_LOCK_STATE            = 27,           // source or target must be WorldObject
                                                            // datalong= 1=lock, 2=unlock, 4=set not-interactable, 8=set interactable
                                                            // datalong2= go entry, datalong3= go search radius
    SCRIPT_COMMAND_STAND_STATE              = 28,           // source = Unit (or WorldObject when creature entry defined), target = Unit (or none)
                                                            // datalong = stand state (enum UnitStandStateType)
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius
                                                            // data_flags = flag_target_as_source           = 0x01
	SCRIPT_COMMAND_MODIFY_NPC_FLAGS         = 29,           // resSource = Creature
                                                            // datalong=NPCFlags
                                                            // datalong2:0x00=toggle, 0x01=add, 0x02=remove
    SCRIPT_COMMAND_SEND_TAXI_PATH           = 30,           // datalong = taxi path id (source or target must be player)
    SCRIPT_COMMAND_TERMINATE_SCRIPT         = 31,           // datalong = search for npc entry if provided
                                                            // datalong2= search distance
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL: terminate steps of this script if npc found
                                                            //                                        ELSE: terminate steps of this script if npc not found
                                                            // dataint=diff to change a waittime of current Waypoint Movement
    SCRIPT_COMMAND_PAUSE_WAYPOINTS          = 32,           // resSource = Creature
                                                            // datalong = 0: unpause waypoint 1: pause waypoint
    SCRIPT_COMMAND_RESERVED_1               = 33,           // reserved for 3.x and later
    SCRIPT_COMMAND_TERMINATE_COND           = 34,           // datalong = condition_id, datalong2 = if != 0 then quest_id of quest that will be failed for player's group if the script is terminated
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL terminate when condition is false ELSE terminate when condition is true
    SCRIPT_COMMAND_SEND_AI_EVENT            = 35,           // resSource = Creature, resTarget = Unit
                                                            // datalong = AIEventType
                                                            // datalong2 = radius. If radius isn't provided and the target is a creature, then send AIEvent to target
    SCRIPT_COMMAND_SET_FACING               = 36,           // resSource = Creature, resTarget WorldObject. Turn resSource towards Taget
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL also set TargetGuid of resSource to resTarget. In this case resTarget MUST be Creature/ Player
                                                            // datalong != 0 Reset TargetGuid, Reset orientation
    SCRIPT_COMMAND_MOVE_DYNAMIC             = 37,           // resSource = Creature, resTarget Worldobject.
                                                            // datalong = 0: Move resSource towards resTarget
                                                            // datalong != 0: Move resSource to a random point between datalong2..datalong around resTarget.
                                                            //      orientation != 0: Obtain a random point around resTarget in direction of orientation
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL Obtain a random point around resTarget in direction of resTarget->GetOrientation + orientation
                                                            // for resTarget == resSource and orientation == 0 this will mean resSource moving forward
    SCRIPT_COMMAND_SEND_MAIL                = 38,           // resSource WorldObject, can be nullptr, resTarget Player
                                                            // datalong: Send mailTemplateId from resSource (if provided) to player resTarget
                                                            // datalong2: AlternativeSenderEntry. Use as sender-Entry
                                                            // dataint1: Delay (>= 0) in Seconds
    SCRIPT_COMMAND_SET_FLY                  = 39,           // resSource = Creature
                                                            // datalong = bool 0=off, 1=on
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL set/unset byte flag UNIT_BYTE1_FLAG_ALWAYS_STAND
    SCRIPT_COMMAND_DESPAWN_GO               = 40,           // resTarget = GameObject
    SCRIPT_COMMAND_RESPAWN                  = 41,           // resSource = Creature. Requires SCRIPT_FLAG_BUDDY_IS_DESPAWNED to find dead or despawned targets
    SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS      = 42,           // resSource = Creature
                                                            // datalong = resetDefault: bool 0=false, 1=true
                                                            // dataint = main hand slot; dataint2 = off hand slot; dataint3 = ranged slot
    SCRIPT_COMMAND_RESET_GO                 = 43,           // resTarget = GameObject
    SCRIPT_COMMAND_UPDATE_TEMPLATE          = 44,           // resSource = Creature
                                                            // datalong = new Creature entry
                                                            // datalong2 = Alliance(0) Horde(1), other values throw error
};


enum ScriptInfoDataFlags
{
	// default: s/b -> t
	SCRIPT_FLAG_BUDDY_AS_TARGET = 0x01,         // s -> b
	SCRIPT_FLAG_REVERSE_DIRECTION = 0x02,         // t* -> s* (* result after previous flag is evaluated)
	SCRIPT_FLAG_SOURCE_TARGETS_SELF = 0x04,         // s* -> s* (* result after previous flag is evaluated)
	SCRIPT_FLAG_COMMAND_ADDITIONAL = 0x08,         // command dependend
	SCRIPT_FLAG_BUDDY_BY_GUID = 0x10,         // take the buddy by guid
	SCRIPT_FLAG_BUDDY_IS_PET = 0x20,         // buddy is a pet
	SCRIPT_FLAG_BUDDY_IS_DESPAWNED = 0X40,         // buddy is dead or despawned
};
#define MAX_SCRIPT_FLAG_VALID               (2 * SCRIPT_FLAG_BUDDY_IS_DESPAWNED - 1)

#define MAX_TEXT_ID 4                                       // used for SCRIPT_COMMAND_TALK

enum SummonCreatureFlags
{
    SUMMON_CREATURE_ACTIVE      = 0x1,
    SUMMON_CREATURE_UNIQUE      = 0x2,                      // not actually unique, just checks for same entry in certain range
    SUMMON_CREATURE_UNIQUE_TEMP = 0x4,                      // same as 0x2 but check for TempSummon only creatures
};

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;

    union
    {
        struct                                              // SCRIPT_COMMAND_TALK (0)
        {
            uint32 chatType;                                // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 language;                                // datalong4
            uint32 flags;                                   // data_flags
            int32  textId[MAX_TEXT_ID];                     // dataint to dataint4
        } talk;

        struct                                              // SCRIPT_COMMAND_EMOTE (1)
        {
            uint32 emoteId;                                 // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 unused1;                                 // datalong4
            uint32 flags;                                   // data_flags
        } emote;

        struct                                              // SCRIPT_COMMAND_FIELD_SET (2)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setField;

        struct                                              // SCRIPT_COMMAND_MOVE_TO (3)
        {
            uint32 unused1;                                 // datalong
            uint32 travelTime;                              // datalong2
        } moveTo;

        struct                                              // SCRIPT_COMMAND_FLAG_SET (4)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setFlag;

        struct                                              // SCRIPT_COMMAND_FLAG_REMOVE (5)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } removeFlag;

        struct                                              // SCRIPT_COMMAND_TELEPORT_TO (6)
        {
            uint32 mapId;                                   // datalong
        } teleportTo;

        struct                                              // SCRIPT_COMMAND_QUEST_EXPLORED (7)
        {
            uint32 questId;                                 // datalong
            uint32 distance;                                // datalong2
        } questExplored;

        struct                                              // SCRIPT_COMMAND_KILL_CREDIT (8)
        {
            uint32 creatureEntry;                           // datalong
            uint32 isGroupCredit;                           // datalong2
        } killCredit;

        struct                                              // SCRIPT_COMMAND_RESPAWN_GAMEOBJECT (9)
        {
            uint32 goGuid;                                  // datalong
            int32 despawnDelay;                             // datalong2
        } respawnGo;

        struct                                              // SCRIPT_COMMAND_TEMP_SUMMON_CREATURE (10)
        {
            uint32 creatureEntry;                           // datalong
            uint32 despawnDelay;                            // datalong2
            uint32 uniqueLimit;                             // datalong3
            uint32 uniqueDistance;                          // datalong4
            uint32 flags;                                   // data_flags
        } summonCreature;

        struct                                              // SCRIPT_COMMAND_OPEN_DOOR (11)
        {
            uint32 goGuid;                                  // datalong
            int32 resetDelay;                               // datalong2
        } openDoor;

        struct                                              // SCRIPT_COMMAND_CLOSE_DOOR (12)
        {
            uint32 goGuid;                                  // datalong
            int32 resetDelay;                               // datalong2
        } closeDoor;

                                                            // SCRIPT_COMMAND_ACTIVATE_OBJECT (13)

        struct                                              // SCRIPT_COMMAND_REMOVE_AURA (14)
        {
            uint32 spellId;                                 // datalong
            uint32 isSourceTarget;                          // datalong2
        } removeAura;

        struct                                              // SCRIPT_COMMAND_CAST_SPELL (15)
        {
            uint32 spellId;                                 // datalong
            uint32 flags;                                   // datalong2
        } castSpell;

        struct                                              // SCRIPT_COMMAND_PLAY_SOUND (16)
        {
            uint32 soundId;                                 // datalong
            uint32 flags;                                   // datalong2
        } playSound;

        struct                                              // SCRIPT_COMMAND_CREATE_ITEM (17)
        {
            uint32 itemEntry;                               // datalong
            uint32 amount;                                  // datalong2
        } createItem;

        struct                                              // SCRIPT_COMMAND_DESPAWN_SELF (18)
        {
            uint32 despawnDelay;                            // datalong
        } despawn;

        struct                                              // SCRIPT_COMMAND_PLAY_MOVIE (19)
        {
            uint32 movieId;                                 // datalong
        } playMovie;

        struct                                              // SCRIPT_COMMAND_MOVEMENT (20)
        {
            uint32 movementType;                            // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } movement;

        struct                                              // SCRIPT_COMMAND_SET_ACTIVEOBJECT (21)
        {
            uint32 activate;                                // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } activeObject;

        struct                                              // SCRIPT_COMMAND_SET_FACTION (22)
        {
            uint32 factionId;                               // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty1;                                  // datalong4
            uint32 flags;                                   // data_flags
        } faction;

        struct                                              // SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL (23)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty1;                                  // datalong4
            uint32 flags;                                   // data_flags
        } morph;

        struct                                              // SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL (24)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty1;                                  // datalong4
            uint32 flags;                                   // data_flags
        } mount;

        struct                                              // SCRIPT_COMMAND_SET_RUN (25)
        {
            uint32 run;                                     // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } run;

        struct                                              // SCRIPT_COMMAND_ATTACK_START (26)
        {
            uint32 empty1;                                  // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty2;                                  // datalong4
            uint32 flags;                                   // data_flags
        } attack;

        struct                                              // SCRIPT_COMMAND_GO_LOCK_STATE (27)
        {
            uint32 lockState;                               // datalong
            uint32 goEntry;                                 // datalong2
            uint32 searchRadius;                            // datalong3
        } goLockState;

        struct                                              // SCRIPT_COMMAND_STAND_STATE (28)
        {
            uint32 stand_state;                             // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 unused1;                                 // datalong4
            uint32 flags;                                   // data_flags
        } standState;
		struct                                              // SCRIPT_COMMAND_MODIFY_NPC_FLAGS (29)
		{
			uint32 flag;                                    // datalong
			uint32 change_flag;                             // datalong2
		} npcFlag;

		struct                                              // SCRIPT_COMMAND_SEND_TAXI_PATH (30)
		{
			uint32 taxiPathId;                              // datalong
			uint32 empty;
		} sendTaxiPath;

		struct                                              // SCRIPT_COMMAND_TERMINATE_SCRIPT (31)
		{
			uint32 npcEntry;                                // datalong
			uint32 searchDist;                              // datalong2
															// changeWaypointWaitTime                       // dataint
		} terminateScript;

		struct                                              // SCRIPT_COMMAND_PAUSE_WAYPOINTS (32)
		{
			uint32 doPause;                                 // datalong
			uint32 empty;
		} pauseWaypoint;

		struct                                              // SCRIPT_COMMAND_TERMINATE_COND (34)
		{
			uint32 conditionId;                             // datalong
			uint32 failQuest;                               // datalong2
		} terminateCond;

		struct                                              // SCRIPT_COMMAND_SEND_AI_EVENT_AROUND (35)
		{
			uint32 eventType;                               // datalong
			uint32 radius;                                  // datalong2
		} sendAIEvent;

		struct                                              // SCRIPT_COMMAND_SET_FACING (36)
		{
			uint32 resetFacing;                             // datalong
			uint32 empty;                                   // datalong2
		} setFacing;

		struct                                              // SCRIPT_COMMAND_MOVE_DYNAMIC (37)
		{
			uint32 maxDist;                                 // datalong
			uint32 minDist;                                 // datalong2
		} moveDynamic;

		struct                                              // SCRIPT_COMMAND_SEND_MAIL (38)
		{
			uint32 mailTemplateId;                          // datalong
			uint32 altSender;                               // datalong2;
		} sendMail;

		struct                                              // SCRIPT_COMMAND_SET_FLY (39)
		{
			uint32 fly;                                     // datalong
			uint32 empty;                                   // datalong2
		} fly;

		// datalong unsed                                   // SCRIPT_COMMAND_DESPAWN_GO (40)
		// datalong unsed                                   // SCRIPT_COMMAND_RESPAWN (41)

		struct                                              // SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS (42)
		{
			uint32 resetDefault;                            // datalong
			uint32 empty;                                   // datalong2
		} setEquipment;

		// datalong unsed                                   // SCRIPT_COMMAND_RESET_GO (43)

		struct                                              // SCRIPT_COMMAND_UPDATE_TEMPLATE (44)
		{
			uint32 newTemplate;                             // datalong
			uint32 newFactionTeam;                          // datalong2
		} updateTemplate;

        struct
        {
            uint32 data[9];
        } raw;
    };

    float x;
    float y;
    float z;
    float o;

	// Buddy system (entry can be npc or go entry, depending on command)
	uint32 buddyEntry;                                      // buddy_entry
	uint32 searchRadiusOrGuid;                              // search_radius (can also be guid in case of SCRIPT_FLAG_BUDDY_BY_GUID)
	uint8 data_flags;                                       // data_flags

    // helpers
    uint32 GetGOGuid() const
    {
        switch(command)
        {
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT: return respawnGo.goGuid;
            case SCRIPT_COMMAND_OPEN_DOOR: return openDoor.goGuid;
            case SCRIPT_COMMAND_CLOSE_DOOR: return closeDoor.goGuid;
            default: return 0;
        }
    }

	bool IsCreatureBuddy() const
	{
		switch (command)
		{
		case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
		case SCRIPT_COMMAND_OPEN_DOOR:
		case SCRIPT_COMMAND_CLOSE_DOOR:
		case SCRIPT_COMMAND_ACTIVATE_OBJECT:
		case SCRIPT_COMMAND_GO_LOCK_STATE:
		case SCRIPT_COMMAND_DESPAWN_GO:
		case SCRIPT_COMMAND_RESET_GO:
			return false;
		default:
			return true;
		}
	}

	bool HasAdditionalScriptFlag() const
	{
		switch (command)
		{
		case SCRIPT_COMMAND_MOVE_TO:
		case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
		case SCRIPT_COMMAND_CAST_SPELL:
		case SCRIPT_COMMAND_MOVEMENT:
		case SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL:
		case SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL:
		case SCRIPT_COMMAND_TERMINATE_SCRIPT:
		case SCRIPT_COMMAND_TERMINATE_COND:
		case SCRIPT_COMMAND_SET_FACING:
		case SCRIPT_COMMAND_MOVE_DYNAMIC:
		case SCRIPT_COMMAND_SET_FLY:
			return true;
		default:
			return false;
		}
	}
};

struct ScriptAction
{
    ObjectGuid sourceGuid;
    ObjectGuid targetGuid;
    ObjectGuid ownerGuid;                                   // owner of source if source is item
    ScriptInfo const* script;                               // pointer to static script data
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;

extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sSpellScripts;
extern ScriptMapMap sGameObjectScripts;
extern ScriptMapMap sGameObjectTemplateScripts;
extern ScriptMapMap sEventScripts;
extern ScriptMapMap sGossipScripts;
extern ScriptMapMap sCreatureDeathScripts;
extern ScriptMapMap sCreatureMovementScripts;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<ScriptDev2 Text Entry Missing!>"

#define TEXT_SOURCE_RANGE -1000000                          //the amount of entries each text source has available

#define TEXT_SOURCE_TEXT_START      TEXT_SOURCE_RANGE
#define TEXT_SOURCE_TEXT_END        TEXT_SOURCE_RANGE*2 + 1

#define TEXT_SOURCE_CUSTOM_START    TEXT_SOURCE_RANGE*2
#define TEXT_SOURCE_CUSTOM_END      TEXT_SOURCE_RANGE*3 + 1

#define TEXT_SOURCE_GOSSIP_START    TEXT_SOURCE_RANGE*3
#define TEXT_SOURCE_GOSSIP_END      TEXT_SOURCE_RANGE*4 + 1

//Spell targets used by SelectSpell
enum SelectTarget
{
    SELECT_TARGET_DONTCARE = 0,                             //All target types allowed

    SELECT_TARGET_SELF,                                     //Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,                             //Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,                                //Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,                                //AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,                            //Only Single Friend
    SELECT_TARGET_AOE_FRIEND,                               //Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,                               //AoE or Single Friend
};

//Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,                             //All spell effects allowed
    SELECT_EFFECT_DAMAGE,                                   //Spell does damage
    SELECT_EFFECT_HEALING,                                  //Spell does healing
    SELECT_EFFECT_AURA,                                     //Spell applies an aura
};

//TODO: find better namings and definitions.
//N=Neutral, A=Alliance, H=Horde.
//NEUTRAL or FRIEND = Hostility to player surroundings (not a good definition)
//ACTIVE or PASSIVE = Hostility to environment surroundings.
enum eEscortFaction
{
    FACTION_ESCORT_A_NEUTRAL_PASSIVE    = 10,
    FACTION_ESCORT_H_NEUTRAL_PASSIVE    = 33,
    FACTION_ESCORT_N_NEUTRAL_PASSIVE    = 113,

    FACTION_ESCORT_A_NEUTRAL_ACTIVE     = 231,
    FACTION_ESCORT_H_NEUTRAL_ACTIVE     = 232,
    FACTION_ESCORT_N_NEUTRAL_ACTIVE     = 250,

    FACTION_ESCORT_N_FRIEND_PASSIVE     = 290,
    FACTION_ESCORT_N_FRIEND_ACTIVE      = 495,

    FACTION_ESCORT_A_PASSIVE            = 774,
    FACTION_ESCORT_H_PASSIVE            = 775,

    FACTION_ESCORT_N_ACTIVE             = 1986,
    FACTION_ESCORT_H_ACTIVE             = 2046
};

struct ScriptPointMove
{
    uint32 uiCreatureEntry;
    uint32 uiPointId;
    float  fX;
    float  fY;
    float  fZ;
    uint32 uiWaitTime;
};

struct StringTextData
{
    uint32 SoundId;
    uint8  Type;
    uint32 Language;
    uint32 Emote;
};

struct CreatureEscortData
{
    uint32 uiCreatureEntry;
    uint32 uiQuestEntry;
    uint32 uiEscortFaction;
    uint32 uiLastWaypointEntry;
};

struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
};

struct Script
{
    Script() :
        Name(""), pGossipHello(nullptr), pGOGossipHello(nullptr), pQuestAcceptNPC(nullptr),
        pGossipSelect(nullptr), pGOGossipSelect(nullptr),
        pGossipSelectWithCode(nullptr), pGOGossipSelectWithCode(nullptr), pQuestComplete(nullptr),
        pNPCDialogStatus(nullptr), pGODialogStatus(nullptr), pQuestRewardedNPC(nullptr), pQuestRewardedGO(nullptr), pItemHello(nullptr), pGOHello(nullptr), pAreaTrigger(nullptr),
        pProcessEventId(nullptr), pItemQuestAccept(nullptr), pGOQuestAccept(nullptr), pEffectScriptEffectNPC(nullptr),
        pItemUse(nullptr), pEffectDummyCreature(nullptr), pEffectDummyGameObj(nullptr), pEffectDummyItem(nullptr),
        pEffectAuraDummy(nullptr), GOOpen(nullptr), pGOUse(nullptr), 
        GOGetAI(nullptr), GetAI(nullptr), GetInstanceData(nullptr)
    {}

    std::string Name;

    //Methods to be scripted
    bool (*pGossipHello             )(Player*, Creature*);
    bool (*pGOGossipHello           )(Player*, GameObject*);
    bool (*pQuestAcceptNPC          )(Player*, Creature*, Quest const*);
    bool (*pGossipSelect            )(Player*, Creature*, uint32, uint32);
    bool (*pGOGossipSelect          )(Player*, GameObject*, uint32, uint32);
    bool (*pGossipSelectWithCode    )(Player*, Creature*, uint32, uint32, const char*);
    bool (*pGOGossipSelectWithCode  )(Player*, GameObject*, uint32, uint32, const char*);
//    bool (*pQuestSelect             )(Player*, Creature*, const Quest*);
    bool (*pQuestComplete           )(Player*, Creature*, const Quest*);
    uint32 (*pNPCDialogStatus       )(Player*, Creature*);
    uint32 (*pGODialogStatus        )(Player*, GameObject*);
    bool (*pQuestRewardedNPC        )(Player*, Creature*, Quest const*);
    bool (*pQuestRewardedGO         )(Player*, GameObject*, Quest const*);
    bool (*pItemHello               )(Player*, Item*, const Quest*);
    bool (*pGOHello                 )(Player*, GameObject*);
    bool (*pAreaTrigger             )(Player*, const AreaTriggerEntry*);
    bool (*pProcessEventId          )(uint32, Object*, Object*, bool);
    bool (*pItemQuestAccept         )(Player*, Item*, const Quest*);
    bool (*pGOQuestAccept           )(Player*, GameObject*, const Quest*);
//    bool (*pGOChooseReward          )(Player*, GameObject*, const Quest*, uint32);
	bool (*pGOUse                    )(Player*, GameObject*);
	bool (*pEffectScriptEffectNPC)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
	//bool (*pEffectDummyNPC)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
    bool (*pItemUse                 )(Player*, Item*, SpellCastTargets const&);
    bool (*pEffectDummyCreature     )(Unit*, uint32, SpellEffectIndex, Creature*);
    bool (*pEffectDummyGameObj      )(Unit*, uint32, SpellEffectIndex, GameObject*);
    bool (*pEffectDummyItem         )(Unit*, uint32, SpellEffectIndex, Item*);
    bool (*pEffectAuraDummy         )(const Aura*, bool);
    bool (*GOOpen                   )(Player* pUser, GameObject* gobj);
    GameObjectAI* (*GOGetAI         )(GameObject* pGo);

    CreatureAI* (*GetAI)(Creature*);
    InstanceData* (*GetInstanceData)(Map*);

    void RegisterSelf(bool custom = false);
};

class ScriptMgr
{
    public:
        ScriptMgr();
        ~ScriptMgr();

        void LoadGameObjectScripts();
        void LoadQuestEndScripts();
        void LoadQuestStartScripts();
        void LoadEventScripts();
        void LoadSpellScripts();
        void LoadGossipScripts();
        void LoadCreatureMovementScripts();
		void LoadCreatureDeathScripts();

        void LoadDbScriptStrings();

        void LoadScriptNames();
        void LoadAreaTriggerScripts();
        void LoadEventIdScripts();
        
        void FillSpellSummary();

        TSpellSummary* GetSpellSummary() const { return m_spellSummary; }

        uint32 GetAreaTriggerScriptId(uint32 triggerId) const;
        uint32 GetEventIdScriptId(uint32 eventId) const;

        const char* GetScriptName(uint32 id) const { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
        uint32 GetScriptId(const char *name) const;
        uint32 GetScriptIdsCount() const { return m_scriptNames.size(); }
        
        void Initialize();
        void LoadDatabase();

        void LoadScriptTexts();
        void LoadScriptTextsCustom();
        void LoadScriptGossipTexts();
        void LoadScriptWaypoints();
        void LoadEscortData();

        StringTextData const* GetTextData(int32 uiTextId) const
        {
            TextDataMap::const_iterator itr = m_mTextDataMap.find(uiTextId);

            if (itr == m_mTextDataMap.end())
                return nullptr;

            return &itr->second;
        }
        
        CreatureEscortData const* GetEscortData(int32 creature_id) const
        {
            EscortDataMap::const_iterator itr = m_mEscortDataMap.find(creature_id);

            if (itr == m_mEscortDataMap.end())
                return nullptr;

            return &itr->second;
        }

        std::vector<ScriptPointMove> const &GetPointMoveList(uint32 uiCreatureEntry) const
        {
            static std::vector<ScriptPointMove> vEmpty;

            auto itr = m_mPointMoveMap.find(uiCreatureEntry);

            if (itr == m_mPointMoveMap.end())
                return vEmpty;

            return itr->second;
        }

        uint32 IncreaseScheduledScriptsCount() { return (uint32)++m_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount() { return (uint32)--m_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount(size_t count) { return (uint32)(m_scheduledScripts -= count); }
        bool IsScriptScheduled() const { return m_scheduledScripts > 0; }
		static bool CanSpellEffectStartDBScript(uint32 spellId, SpellEffectIndex effIdx);

        CreatureAI* GetCreatureAI(Creature* pCreature);
        GameObjectAI* GetGameObjectAI(GameObject* pGob);
        InstanceData* CreateInstanceData(Map* pMap);

        bool OnGossipHello(Player* pPlayer, Creature* pCreature);
        bool OnGossipHello(Player* pPlayer, GameObject* pGameObject);
        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code);
        bool OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        uint32 GetDialogStatus(Player* pPlayer, Creature* pCreature);
        uint32 GetDialogStatus(Player* pPlayer, GameObject* pGameObject);
        bool OnGameObjectUse(Player* pPlayer, GameObject* pGameObject);
        bool OnGameObjectOpen(Player* pPlayer, GameObject* pGameObject);
        bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
        bool OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry);
        bool OnProcessEvent(uint32 eventId, Object* pSource, Object* pTarget, bool isStart);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget);
        bool OnAuraDummy(Aura const* pAura, bool apply);

    private:
        void CollectPossibleEventIds(std::set<uint32>& eventIds);
        void LoadScripts(ScriptMapMap& scripts, const char* tablename);
        void CheckScriptTexts(ScriptMapMap const& scripts, std::set<int32>& ids);

        typedef std::vector<std::string> ScriptNameMap;
        typedef UNORDERED_MAP<uint32, uint32> AreaTriggerScriptMap;
        typedef UNORDERED_MAP<uint32, uint32> EventIdScriptMap;
        
        //Maps and lists
        typedef UNORDERED_MAP<int32, StringTextData> TextDataMap;
        typedef UNORDERED_MAP<uint32, std::vector<ScriptPointMove> > PointMoveMap;
        typedef UNORDERED_MAP<int32, CreatureEscortData> EscortDataMap;

        AreaTriggerScriptMap    m_AreaTriggerScripts;
        EventIdScriptMap        m_EventIdScripts;

        ScriptNameMap           m_scriptNames;

        TSpellSummary* m_spellSummary;
        
        TextDataMap     m_mTextDataMap;                     //additional data for text strings
        PointMoveMap    m_mPointMoveMap;                    //coordinates for waypoints
        EscortDataMap   m_mEscortDataMap;                   // Des donnees pour les quetes d'escorte scriptees via la DB

        //atomic op counter for active scripts amount
        ACE_Atomic_Op<ACE_Thread_Mutex, int> m_scheduledScripts;
};

//Generic scripting text function
void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target = nullptr);
void DoOrSimulateScriptTextForMap(int32 iTextEntry, uint32 uiCreatureEntry, Map* pMap, Creature* pCreatureSource = nullptr, Unit* pTarget = nullptr);

#define sScriptMgr MaNGOS::Singleton<ScriptMgr>::Instance()

MANGOS_DLL_SPEC uint32 GetAreaTriggerScriptId(uint32 triggerId);
MANGOS_DLL_SPEC uint32 GetEventIdScriptId(uint32 eventId);
MANGOS_DLL_SPEC uint32 GetScriptId(const char *name);
MANGOS_DLL_SPEC char const* GetScriptName(uint32 id);
MANGOS_DLL_SPEC uint32 GetScriptIdsCount();

#endif

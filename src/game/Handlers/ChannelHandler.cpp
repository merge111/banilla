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

#include "ObjectMgr.h"                                      // for normalizePlayerName
#include "ChannelMgr.h"
#include "Chat.h"
#include "World.h"

void WorldSession::HandleJoinChannelOpcode(WorldPacket& recvPacket)
{
    std::string channelname, pass;

    recvPacket >> channelname;

    DEBUG_LOG("Opcode CMSG_JOIN_CHANNEL channel \"%s\"", channelname.c_str());

    if (channelname.empty())
        return;

    recvPacket >> pass;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        cMgr->SetJoinChannel(channelname, player, pass);
    }
    if (player->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_JOIN_OPPOSITE_FACTION_CHANNELS))
    {
        if (ChannelMgr* cMgr = channelMgr(_player->GetTeam() == ALLIANCE ? HORDE : ALLIANCE))
            if (Channel *chn = cMgr->GetChannel(channelname, player))
                if (!chn->GetSecurityLevel()) // Special both factions channel
                    cMgr->SetJoinChannel(channelname, player, pass);
    }
}

void WorldSession::HandleLeaveChannelOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    // uint32 unk;
    std::string channelname;
    // recvPacket >> unk;                                      // channel id?
    recvPacket >> channelname;

    if (channelname.empty())
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(channelname, player))
            chn->Leave(player->GetObjectGuid(), channelname.c_str(), true);
        cMgr->LeftChannel(channelname, player);
    }
    if (player->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_JOIN_OPPOSITE_FACTION_CHANNELS))
        if (ChannelMgr* cMgr = channelMgr(player->GetTeam() == ALLIANCE ? HORDE : ALLIANCE))
        {
            if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
                chn->Leave(player->GetObjectGuid(), channelname.c_str(), true);
            cMgr->LeftChannel(channelname, player);
        }
}

void WorldSession::HandleChannelListOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->List(player, channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelPasswordOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, pass;
    recvPacket >> channelname;

    recvPacket >> pass;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Password(player->GetObjectGuid(), pass.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelSetOwnerOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();

    std::string channelname, newp;
    recvPacket >> channelname;

    recvPacket >> newp;

    if (!normalizePlayerName(newp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->SetOwner(player->GetObjectGuid(), newp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelOwnerOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->SendWhoOwner(player->GetObjectGuid(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelModeratorOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->SetModerator(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelUnmoderatorOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->UnsetModerator(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelMuteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->SetMute(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelUnmuteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->UnsetMute(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelInviteOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (player->getLevel() < sWorld.getConfig(CONFIG_UINT32_CHANNEL_INVITE_MIN_LEVEL))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Invite(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelKickOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;
    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Kick(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelBanOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Ban(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelUnbanOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname, otp;
    recvPacket >> channelname;

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->UnBan(player->GetObjectGuid(), otp.c_str(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelAnnouncementsOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Announce(player->GetObjectGuid(), channelname.c_str());
        else
            ForwardPacketToNode();
}

void WorldSession::HandleChannelModerateOpcode(WorldPacket& recvPacket)
{
    DEBUG_LOG("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    std::string channelname;
    recvPacket >> channelname;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, player, IsNode()))
            chn->Moderate(player->GetObjectGuid(), channelname.c_str());
        else
            ForwardPacketToNode();
}


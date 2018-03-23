/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WorldSession.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Language.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "Random.h"
#include "TicketMgr.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include <zlib.h>

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket& recvData)
{
    // Don't accept tickets if the ticket queue is disabled. (Ticket UI is greyed out but not fully dependable)
    if (sTicketMgr->GetStatus() == GMTICKET_QUEUE_STATUS_DISABLED)
        return;

    if (GetPlayer()->getLevel() < sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ))
    {
        SendNotification(GetTrinityString(LANG_TICKET_REQ), sWorld->getIntConfig(CONFIG_TICKET_LEVEL_REQ));
        return;
    }

    GMTicketResponse response = GMTICKET_RESPONSE_CREATE_ERROR;
    GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID());

    if (ticket && ticket->IsCompleted())
        sTicketMgr->CloseTicket(ticket->GetId(), GetPlayer()->GetGUID());

    // Player must not have ticket
    if (!ticket || ticket->IsClosed())
    {
        uint32 mapId;
        float x, y, z;
        std::string message;
        uint32 needResponse;
        bool needMoreHelp;
        uint32 count;
        std::list<uint32> times;
        uint32 decompressedSize;
        std::string chatLog;

        recvData >> mapId;
        recvData >> x >> y >> z;
        recvData >> message;

        recvData >> needResponse;
        recvData >> needMoreHelp;

        recvData >> count;

        for (uint32 i = 0; i < count; i++)
        {
            uint32 time;
            recvData >> time;
            times.push_back(time);
        }

        recvData >> decompressedSize;

        if (count && decompressedSize && decompressedSize < 0xFFFF)
        {
            uint32 pos = recvData.rpos();
            ByteBuffer dest;
            dest.resize(decompressedSize);

            uLongf realSize = decompressedSize;
            if (uncompress(dest.contents(), &realSize, recvData.contents() + pos, recvData.size() - pos) == Z_OK)
            {
                dest >> chatLog;
            }
            else
            {
                TC_LOG_ERROR("network", "CMSG_GMTICKET_CREATE possibly corrupt. Uncompression failed.");
                recvData.rfinish();
                return;
            }

            recvData.rfinish(); // Will still have compressed data in buffer.
        }

        ticket = new GmTicket(GetPlayer());
        ticket->SetPosition(mapId, x, y, z);
        ticket->SetMessage(message);
        ticket->SetGmAction(needResponse, needMoreHelp);

        if (!chatLog.empty())
            ticket->SetChatLog(times, chatLog);

        sTicketMgr->AddTicket(ticket);
        sTicketMgr->UpdateLastChange();

        sWorld->SendGMText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName().c_str(), ticket->GetId());

        response = GMTICKET_RESPONSE_CREATE_SUCCESS;
    }

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    data << uint32(response);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketUpdateOpcode(WorldPacket& recvData)
{
    std::string message;
    recvData >> message;

    GMTicketResponse response = GMTICKET_RESPONSE_UPDATE_ERROR;
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetMessage(message);
        ticket->SaveToDB(trans);

        sWorld->SendGMText(LANG_COMMAND_TICKETUPDATED, GetPlayer()->GetName().c_str(), ticket->GetId());

        response = GMTICKET_RESPONSE_UPDATE_SUCCESS;
    }

    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);
    data << uint32(response);
    SendPacket(&data);
}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket & /*recvData*/)
{
    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
    {
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(GMTICKET_RESPONSE_TICKET_DELETED);
        SendPacket(&data);

        sWorld->SendGMText(LANG_COMMAND_TICKETPLAYERABANDON, GetPlayer()->GetName().c_str(), ticket->GetId());

        sTicketMgr->CloseTicket(ticket->GetId(), GetPlayer()->GetGUID());
        sTicketMgr->SendTicket(this, nullptr);
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket & /*recvData*/)
{
    SendQueryTimeResponse();

    if (GmTicket* ticket = sTicketMgr->GetTicketByPlayer(GetPlayer()->GetGUID()))
        sTicketMgr->SendTicket(this, ticket);
    else
        sTicketMgr->SendTicket(this, nullptr);
}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket & /*recvData*/)
{
    // Note: This only disables the ticket UI at client side and is not fully reliable
    // are we sure this is a uint32? Should ask Zor
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32(sTicketMgr->GetStatus() ? GMTICKET_QUEUE_STATUS_ENABLED : GMTICKET_QUEUE_STATUS_DISABLED);
    SendPacket(&data);
}

void WorldSession::HandleGMSurveySubmit(WorldPacket& recvData)
{
    uint32 nextSurveyID = sTicketMgr->GetNextSurveyID();
    // just put the survey into the database
    uint32 mainSurvey; // GMSurveyCurrentSurvey.dbc, column 1 (all 9) ref to GMSurveySurveys.dbc
    recvData >> mainSurvey;

    std::unordered_set<uint32> surveyIds;
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // sub_survey1, r1, comment1, sub_survey2, r2, comment2, sub_survey3, r3, comment3, sub_survey4, r4, comment4, sub_survey5, r5, comment5, sub_survey6, r6, comment6, sub_survey7, r7, comment7, sub_survey8, r8, comment8, sub_survey9, r9, comment9, sub_survey10, r10, comment10,
    for (uint8 i = 0; i < 10; i++)
    {
        uint32 subSurveyId; // ref to i'th GMSurveySurveys.dbc field (all fields in that dbc point to fields in GMSurveyQuestions.dbc)
        recvData >> subSurveyId;
        if (!subSurveyId)
            break;

        uint8 rank; // probably some sort of ref to GMSurveyAnswers.dbc
        recvData >> rank;
        std::string comment; // comment ("Usage: GMSurveyAnswerSubmit(question, rank, comment)")
        recvData >> comment;

        // make sure the same sub survey is not added to DB twice
        if (!surveyIds.insert(subSurveyId).second)
            continue;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SUBSURVEY);
        stmt->setUInt32(0, nextSurveyID);
        stmt->setUInt32(1, subSurveyId);
        stmt->setUInt32(2, rank);
        stmt->setString(3, comment);
        trans->Append(stmt);
    }

    std::string comment; // just a guess
    recvData >> comment;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SURVEY);
    stmt->setUInt32(0, GetPlayer()->GetGUID().GetCounter());
    stmt->setUInt32(1, nextSurveyID);
    stmt->setUInt32(2, mainSurvey);
    stmt->setString(3, comment);

    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

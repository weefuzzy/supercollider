/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "SC_Lib.h"
#include "SC_ComPort.h"
#include "SC_CoreAudio.h"
#include "SC_HiddenWorld.h"
#include "SC_Graph.h"
#include "SC_GraphDef.h"
#include "SC_Group.h"
#include "SC_UnitDef.h"
#include <stdexcept>
#include "SC_Lib_Cintf.h"
#include "SC_SequencedCommand.h"
#include <new>
#include "SC_Prototypes.h"
#include "scsynthsend.h"

// returns number of bytes in an OSC string.
int OSCstrlen(char *strin);

Node* Msg_GetNode(World *inWorld, sc_msg_iter& msg)
{
	Node *node;
	if (msg.nextTag('i') == 's')
	{
		char* loc = msg.gets();
		int32 nodeID = msg.geti();
		node = World_GetNode(inWorld, nodeID);
		while (*loc)
		{
			if (!node) return 0;
			switch (*loc)
			{
				case 'h' :
					if (!node->mIsGroup) return 0;
					node = ((Group*)node)->mHead;
					break;
				case 't' :
					if (!node->mIsGroup) return 0;
					node = ((Group*)node)->mTail;
					break;
				case 'u' :
					node = &node->mParent->mNode;
					break;
				case 'p' :
					node = node->mPrev;
					break;
				case 'n' :
					node = node->mNext;
					break;
			}
			loc++;
		}
	}
	else 
	{
		int32 nodeID = msg.geti();
		node = World_GetNode(inWorld, nodeID);
	}
	return node;
}

Group* Msg_GetGroup(World *inWorld, sc_msg_iter& msg)
{
	Node* node = Msg_GetNode(inWorld, msg);
	return node->mIsGroup ? (Group*)node : 0;
}

Graph* Msg_GetGraph(World *inWorld, sc_msg_iter& msg)
{
	Node* node = Msg_GetNode(inWorld, msg);
	return node->mIsGroup ? 0 : (Graph*)node;
}

SCErr meth_none(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_none(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{	
	return kSCErr_None;
}

SCErr meth_b_alloc(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_alloc(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{		
	CallSequencedCommand(BufAllocCmd, inWorld, inSize, inData, inReply);
	return kSCErr_None;
}

SCErr meth_b_free(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_free(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{	
	CallSequencedCommand(BufFreeCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;
}

SCErr meth_b_close(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_close(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(BufCloseCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;
}

SCErr meth_b_allocRead(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_allocRead(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{	
	CallSequencedCommand(BufAllocReadCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;
}

SCErr meth_b_read(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_read(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(BufReadCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;
}

SCErr meth_b_write(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_write(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(BufWriteCmd, inWorld, inSize, inData, inReply);

	return kSCErr_None;
}

SCErr meth_b_zero(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_zero(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(BufZeroCmd, inWorld, inSize, inData, inReply);
	return kSCErr_None;
}


SCErr meth_u_cmd(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_u_cmd(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	return Unit_DoCmd(inWorld, inSize, inData);	
};

SCErr meth_cmd(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_cmd(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	return PlugIn_DoCmd(inWorld, inSize, inData, inReply);	
};


/*
SCErr meth_n_cmd(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_cmd(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;
		
	char *args = msg.rdpos;
	int arglen = msg.remain();
	
//!!	(node->mDef->fNodeCmd)(node, arglen, args);

	return kSCErr_None;
}
*/

SCErr meth_n_trace(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_trace(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
		
		Node_Trace(node);
	}
	
	return kSCErr_None;
}

SCErr meth_n_run(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_run(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	
	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
			
		int32 run = msg.geti();
	
		Node_SetRun(node, run);
	}
	
	return kSCErr_None;
}

SCErr meth_n_map(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_map(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;
	
	while (msg.remain() >= 8) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int bus = msg.geti();
			Node_MapControl(node, Hash(name), name, 0, bus);
		} else {
			int32 index = msg.geti();
			int32 bus = msg.geti();
			Node_MapControl(node, index, bus);
		}
	}
	return kSCErr_None;
}

SCErr meth_n_mapn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_mapn(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;
	
	while (msg.remain() >= 12) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int32 hash = Hash(name);
			int bus = msg.geti();
			int n = msg.geti();
			for (int i=0; i<n; ++i) {
				Node_MapControl(node, hash, name, i, bus == -1 ? -1 : bus+i);
			}
		} else {
			int32 index = msg.geti();
			int32 bus = msg.geti();
			int n = msg.geti();
			for (int i=0; i<n; ++i) {
				Node_MapControl(node, index+i, bus == -1 ? -1 : bus+i);
			}
		}
	}
	return kSCErr_None;
}

SCErr meth_n_set(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_set(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;
	
	while (msg.remain() >= 8) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			float32 value = msg.getf();
			Node_SetControl(node, Hash(name), name, 0, value);
		} else {
			int32 index = msg.geti();
			float32 value = msg.getf();
			Node_SetControl(node, index, value);
		}
	}
	return kSCErr_None;
}

SCErr meth_n_setn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_setn(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;

	while (msg.remain()) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int32 hash = Hash(name);
			int32 n = msg.geti();
			for (int i=0; msg.remain() && i<n; ++i) {
				float32 value = msg.getf();
				Node_SetControl(node, hash, name, i, value);
			}
		} else {
			int32 index = msg.geti();
			int32 n = msg.geti();
			for (int i=0; msg.remain() && i<n; ++i) {
				float32 value = msg.getf();
				Node_SetControl(node, index+i, value);
			}
		}
	}
	return kSCErr_None;
}

SCErr meth_n_fill(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_fill(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	Node *node = Msg_GetNode(inWorld, msg);
	if (!node) return kSCErr_NodeNotFound;

	while (msg.remain() >= 12) 
	{
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int32 hash = Hash(name);
			int32 n = msg.geti();
			float32 value = msg.getf();
			
			for (int i=0; i<n; ++i) {
				Node_SetControl(node, hash, name, i, value);
			}
		} else {
			int32 index = msg.geti();
			int32 n = msg.geti();
			float32 value = msg.getf();
			
			for (int i=0; i<n; ++i) {
				Node_SetControl(node, index+i, value);
			}
		}
	}

	return kSCErr_None;
}


SCErr meth_n_query(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_query(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	sc_msg_iter msg(inSize, inData);	
	
	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Node_StateMsg(node, kNode_Info);
	}
	return kSCErr_None;
}

SCErr meth_b_query(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_query(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	sc_msg_iter msg(inSize, inData);	
	
	scpacket packet;

	int numbufs = msg.remain() >> 2;
	packet.adds("/b_info");
	packet.maketags(numbufs * 4 + 1);
	packet.addtag(',');
	
	while (msg.remain()) {
		int bufindex = msg.geti();
		SndBuf* buf = World_GetBuf(inWorld, bufindex);

		packet.addtag('i');
		packet.addtag('i');
		packet.addtag('i');
		packet.addtag('f');
		packet.addi(bufindex);
		packet.addi(buf->frames);
		packet.addi(buf->channels);
		packet.addf(buf->samplerate);
	}
	
	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}
	
	return kSCErr_None;
}



SCErr meth_d_load(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_d_load(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(LoadSynthDefCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;

}

SCErr meth_d_recv(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_d_recv(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(RecvSynthDefCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;

}

SCErr meth_d_loadDir(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_d_loadDir(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(LoadSynthDefDirCmd, inWorld, inSize, inData, inReply);
	
	return kSCErr_None;
}

SCErr meth_d_freeAll(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_d_freeAll(World *inWorld, int /*inSize*/, char */*inData*/, ReplyAddress* /*inReply*/)
{
	World_FreeAllGraphDefs(inWorld);
	return kSCErr_None;
}


SCErr meth_s_new(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_s_new(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	SCErr err;
	sc_msg_iter msg(inSize, inData);	
	int32 *defname = msg.gets4();
	if (!defname) return kSCErr_WrongArgType;
	
	int32 nodeID = msg.geti();
	int32 addAction = msg.geti();

	GraphDef *def = World_GetGraphDef(inWorld, defname);
	if (!def) return kSCErr_SynthDefNotFound;
	

	Graph *graph = 0;
	switch (addAction) {
		case 0 : {
			Group *group = Msg_GetGroup(inWorld, msg);
			if (!group) return kSCErr_GroupNotFound;
			err = Graph_New(inWorld, def, nodeID, &msg, &graph);
			if (err) return err;
			if (!graph) return kSCErr_Failed;
			Group_AddHead(group, &graph->mNode);
		} break;
		case 1 : {
			Group *group = Msg_GetGroup(inWorld, msg);
			if (!group) return kSCErr_GroupNotFound;
			err = Graph_New(inWorld, def, nodeID, &msg, &graph);
			if (err) return err;
			Group_AddTail(group, &graph->mNode);
		} break;
		case 2 : {
			Node *beforeThisNode = Msg_GetNode(inWorld, msg);
			if (!beforeThisNode) return kSCErr_NodeNotFound;
			err = Graph_New(inWorld, def, nodeID, &msg, &graph);
			if (err) return err;
			Node_AddBefore(&graph->mNode, beforeThisNode);
		} break;
		case 3 : {
			Node *afterThisNode = Msg_GetNode(inWorld, msg);
			if (!afterThisNode) return kSCErr_NodeNotFound;
			err = Graph_New(inWorld, def, nodeID, &msg, &graph);
			if (err) return err;
			Node_AddAfter(&graph->mNode, afterThisNode);
		} break;
		case 4 : {
			Node *replaceThisNode = Msg_GetNode(inWorld, msg);
			if (!replaceThisNode) return kSCErr_NodeNotFound;
			err = Graph_New(inWorld, def, nodeID, &msg, &graph);
			if (err) return err;
			Node_Replace(&graph->mNode, replaceThisNode);
		} break;
		default: return kSCErr_Failed;
	}
	Node_StateMsg(&graph->mNode, kNode_Go);
	return kSCErr_None;
}

SCErr meth_g_new(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_new(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	SCErr err;
	
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		int32 newGroupID = msg.geti();
		int32 addAction = msg.geti();
			
		Group *newGroup = 0;
		switch (addAction) {
			case 0 : {
				Group *group = Msg_GetGroup(inWorld, msg);
				if (!group) return kSCErr_GroupNotFound;
				err = Group_New(inWorld, newGroupID, &newGroup);
				if (err) {
					if (err == kSCErr_DuplicateNodeID) {
						newGroup = World_GetGroup(inWorld, newGroupID);
						if (!newGroup || !newGroup->mNode.mParent || newGroup->mNode.mParent != group)
							return err;
					} else return err;
				}
				Group_AddHead(group, &newGroup->mNode);
			} break;
			case 1 : {
				Group *group = Msg_GetGroup(inWorld, msg);
				if (!group) return kSCErr_GroupNotFound;
				err = Group_New(inWorld, newGroupID, &newGroup);
				if (err) {
					if (err == kSCErr_DuplicateNodeID) {
						newGroup = World_GetGroup(inWorld, newGroupID);
						if (!newGroup || !newGroup->mNode.mParent || newGroup->mNode.mParent != group)
							return err;
					} else return err;
				}
				Group_AddTail(group, &newGroup->mNode);
			} break;
			case 2 : {
				Node *beforeThisNode = Msg_GetNode(inWorld, msg);
				if (!beforeThisNode) return kSCErr_TargetNodeNotFound;
				err = Group_New(inWorld, newGroupID, &newGroup);
				if (err) {
					if (err == kSCErr_DuplicateNodeID) {
						newGroup = World_GetGroup(inWorld, newGroupID);
						if (!newGroup || !newGroup->mNode.mParent || newGroup->mNode.mParent->mNode.mID != beforeThisNode->mParent->mNode.mID)
							return err;
					} else return err;
				}
				Node_AddBefore(&newGroup->mNode, beforeThisNode);
			} break;
			case 3 : {
				Node *afterThisNode = Msg_GetNode(inWorld, msg);
				if (!afterThisNode) return kSCErr_TargetNodeNotFound;
				err = Group_New(inWorld, newGroupID, &newGroup);
				if (err) {
					if (err == kSCErr_DuplicateNodeID) {
						newGroup = World_GetGroup(inWorld, newGroupID);
						if (!newGroup || !newGroup->mNode.mParent || newGroup->mNode.mParent->mNode.mID != afterThisNode->mParent->mNode.mID)
							return err;
					} else return err;
				}
				Node_AddAfter(&newGroup->mNode, afterThisNode);
			} break;
			case 4 : {
				Node *replaceThisNode = Msg_GetNode(inWorld, msg);
				if (!replaceThisNode) return kSCErr_TargetNodeNotFound;
				err = Group_New(inWorld, newGroupID, &newGroup);
				if (err) return err;
				Node_Replace(&newGroup->mNode, replaceThisNode);
			} break;
			default: return kSCErr_Failed;
		}
	
		Node_StateMsg(&newGroup->mNode, kNode_Go);
	}
	
	return kSCErr_None;
}


SCErr meth_n_free(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_free(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	

	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Node_Delete(node);
	}
	
	return kSCErr_None;
}

SCErr meth_n_before(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_before(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Node *beforeThisOne = Msg_GetNode(inWorld, msg);
		if (!beforeThisOne) return kSCErr_NodeNotFound;
	
		//Group *prevGroup = node->mParent;
			
		Node_Remove(node);
		Node_AddBefore(node, beforeThisOne);
	
		//if (node->mParent != prevGroup) {
			Node_StateMsg(node, kNode_Move);
		//}
	}
	return kSCErr_None;
}

SCErr meth_n_after(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_n_after(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Node *afterThisOne = Msg_GetNode(inWorld, msg);
		if (!afterThisOne) return kSCErr_NodeNotFound;
	
		Group *prevGroup = node->mParent;
		
		Node_Remove(node);
		Node_AddBefore(node, afterThisOne);
	
		if (node->mParent != prevGroup) {
			Node_StateMsg(node, kNode_Move);
		}
	}
	return kSCErr_None;
}

SCErr meth_g_head(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_head(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Group *group = Msg_GetGroup(inWorld, msg);
		if (!group) return kSCErr_GroupNotFound;
	
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Group *prevGroup = node->mParent;
	
		Node_Remove(node);
		
		Group_AddHead(group, node);
		
		if (group != prevGroup) {
			Node_StateMsg(node, kNode_Move);
		}
	}
	return kSCErr_None;
}

SCErr meth_g_tail(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_tail(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Group *group = Msg_GetGroup(inWorld, msg);
		if (!group) return kSCErr_GroupNotFound;
	
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		//Group *prevGroup = node->mParent;
	
		Node_Remove(node);
		Group_AddTail(group, node);
	
		//if (group != prevGroup) {
			Node_StateMsg(node, kNode_Move);
		//}
	}
	return kSCErr_None;
}

SCErr meth_g_insert(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_insert(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Group *group = Msg_GetGroup(inWorld, msg);
		if (!group) return kSCErr_GroupNotFound;
	
		Node *node = Msg_GetNode(inWorld, msg);
		if (!node) return kSCErr_NodeNotFound;
	
		Group *prevGroup = node->mParent;
	
		int index = msg.geti();
		
		Node_Remove(node);
		Group_Insert(group, node, index);
	
		if (group != prevGroup) {
			Node_StateMsg(node, kNode_Move);
		}
	}
	return kSCErr_None;
}

SCErr meth_g_freeAll(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_freeAll(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Group *group = Msg_GetGroup(inWorld, msg);
		if (!group) return kSCErr_GroupNotFound;
	
		Group_DeleteAll(group);
	}
	return kSCErr_None;
}

SCErr meth_g_deepFree(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_g_deepFree(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);	
	while (msg.remain()) {
		Group *group = Msg_GetGroup(inWorld, msg);
		if (!group) return kSCErr_GroupNotFound;
	
		Group_DeepFreeGraphs(group);
	}
	return kSCErr_None;
}



SCErr meth_status(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_status(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(AudioStatusCmd, inWorld, inSize, inData, inReply);
	return kSCErr_None;
}

SCErr meth_quit(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_quit(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(AudioQuitCmd, inWorld, inSize, inData, inReply);
	return kSCErr_None;
}

SCErr meth_clearSched(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_clearSched(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	inWorld->hw->mAudioDriver->ClearSched();
	return kSCErr_None;
}

SCErr meth_dumpOSC(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_dumpOSC(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	sc_msg_iter msg(inSize, inData);
	inWorld->mDumpOSC = msg.geti();
	return kSCErr_None;
}

SCErr meth_b_set(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_set(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);
	int bufindex = msg.geti();
	SndBuf* buf = World_GetBuf(inWorld, bufindex);
	if (!buf) return kSCErr_Failed;
	
	float *data = buf->data;
	uint32 numSamples = buf->samples;
	
	while (msg.remain() >= 8)
	{	
		uint32 sampleIndex = msg.geti();
		float32 value = msg.getf();
		if (sampleIndex < numSamples) 
		{
			data[sampleIndex] = value;
		} else return kSCErr_IndexOutOfRange;
		
	}
	return kSCErr_None;
}

SCErr meth_b_setn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_setn(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{		
	sc_msg_iter msg(inSize, inData);
	int bufindex = msg.geti();
	SndBuf* buf = World_GetBuf(inWorld, bufindex);
	if (!buf) return kSCErr_Failed;
	
	float *data = buf->data;
	int numSamples = buf->samples;
			
	while (msg.remain()) {
		int32 start = msg.geti();
		int32 n = msg.geti();
		int32 end = start+n-1;
		
		if (end < 0 || start >= numSamples) continue;
		
		start = sc_clip(start, 0, numSamples-1);
		end   = sc_clip(end,   0, numSamples-1);

		for (int i=start; msg.remain() && i<=end; ++i) {
			float32 value = msg.getf();
			data[i] = value;
		}
	}

	return kSCErr_None;
}


SCErr meth_b_fill(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_fill(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{		
	sc_msg_iter msg(inSize, inData);
	int bufindex = msg.geti();
	SndBuf* buf = World_GetBuf(inWorld, bufindex);
	if (!buf) return kSCErr_Failed;
	
	float *data = buf->data;
	int numSamples = buf->samples;

	while (msg.remain() >= 12) 
	{
		int32 start = msg.geti();
		int32 n = msg.geti();
		float32 value = msg.getf();
		int32 end = start+n-1;
		
		if (end < 0 || start >= numSamples) continue;
		
		start = sc_clip(start, 0, numSamples-1);
		end   = sc_clip(end,   0, numSamples-1);
		
		for (int i=start; i<=end; ++i) data[i] = value;
	}

	return kSCErr_None;
}

SCErr meth_b_gen(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_gen(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{		
	CallSequencedCommand(BufGenCmd, inWorld, inSize, inData, inReply);

	return kSCErr_None;
}


SCErr meth_c_set(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_c_set(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{
	sc_msg_iter msg(inSize, inData);
	
	float *data = inWorld->mControlBus;
	int32 *touched = inWorld->mControlBusTouched;
	int32 bufCounter = inWorld->mBufCounter;
	uint32 maxIndex = inWorld->mNumControlBusChannels;
	
	while (msg.remain() >= 8)
	{	
		uint32 index = msg.geti();
		float32 value = msg.getf();
		if (index < maxIndex) 
		{
			data[index] = value;
			touched[index] = bufCounter;
		} else return kSCErr_IndexOutOfRange;
		
	}
	return kSCErr_None;
}

SCErr meth_c_setn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_c_setn(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{		
	sc_msg_iter msg(inSize, inData);
	
	float *data = inWorld->mControlBus;
	int32 *touched = inWorld->mControlBusTouched;
	int32 bufCounter = inWorld->mBufCounter;
	int maxIndex = inWorld->mNumControlBusChannels;
	
	while (msg.remain()) {
		int32 start = msg.geti();
		int32 n = msg.geti();
		int32 end = start+n-1;
		
		if (start < 0 || end >= maxIndex || start > end) 
			return kSCErr_IndexOutOfRange;

		for (int i=start; msg.remain() && i<=end; ++i) {
			float32 value = msg.getf();
			data[i] = value;
			touched[i] = bufCounter;
		}
	}

	return kSCErr_None;
}


SCErr meth_c_get(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_c_get(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{
	sc_msg_iter msg(inSize, inData);
		
	float *data = inWorld->mControlBus;
	uint32 maxIndex = inWorld->mNumControlBusChannels;
	
	int numheads = msg.remain() >> 2;

	scpacket packet;
	packet.adds("/c_set");
	packet.maketags(numheads * 2 + 1);
	packet.addtag(',');
	
	while (msg.remain() >= 4)
	{	
		uint32 index = msg.geti();
		if (index >= maxIndex)
			return kSCErr_IndexOutOfRange;
		packet.addtag('i');
		packet.addtag('f');
		packet.addi(index);
		packet.addf(data[index]);
	}
	
	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}
	
	return kSCErr_None;
}

SCErr meth_c_getn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_c_getn(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{		
	sc_msg_iter msg(inSize, inData);
	
	float *data = inWorld->mControlBus;
	int maxIndex = inWorld->mNumControlBusChannels;
	
	// figure out how many tags to allocate   
	int numcontrols = 0;
	int numheads = msg.remain() >> 3;
	
	while (msg.remain()) {
		msg.geti(); // skip start
		int32 n = msg.geti();
		numcontrols += n;
	}
	
	scpacket packet;
	packet.adds("/c_setn");
	packet.maketags(numheads * 2 + numcontrols + 1);
	packet.addtag(',');

	// start over at beginning of message
	msg.init(inSize, inData);
	
	while (msg.remain()) {
		int32 start = msg.geti();
		int32 n = msg.geti();
		int32 end = start+n-1;
		
		if (start < 0 || end >= maxIndex || start > end) 
			return kSCErr_IndexOutOfRange;
		
		packet.addtag('i');
		packet.addtag('i');
		packet.addi(start);
		packet.addi(n);

		for (int i=start; i<=end; ++i) {
			packet.addtag('f');
			packet.addf(data[i]);
		}
	}

	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}

	return kSCErr_None;
}


SCErr meth_c_fill(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_c_fill(World *inWorld, int inSize, char *inData, ReplyAddress* /*inReply*/)
{		
	sc_msg_iter msg(inSize, inData);
	
	float *data = inWorld->mControlBus;
	int32 *touched = inWorld->mControlBusTouched;
	int32 bufCounter = inWorld->mBufCounter;
	int maxIndex = inWorld->mNumControlBusChannels;

	while (msg.remain() >= 12) 
	{
		int32 start = msg.geti();
		int32 n = msg.geti();
		float32 value = msg.getf();
		int32 end = start+n-1;
		
		if (end < 0 || start >= maxIndex) continue;
		
		start = sc_clip(start, 0, maxIndex-1);
		end   = sc_clip(end,   0, maxIndex-1);
		
		for (int i=start; i<=end; ++i) {
			data[i] = value;
			touched[i] = bufCounter;
		}
	}

	return kSCErr_None;
}



SCErr meth_b_get(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_get(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{
	sc_msg_iter msg(inSize, inData);
	int bufindex = msg.geti();
	SndBuf* buf = World_GetBuf(inWorld, bufindex);
	if (!buf) return kSCErr_Failed;
		
	float *data = buf->data;
	uint32 maxIndex = buf->samples;
	
	int numheads = msg.remain() >> 2;

	scpacket packet;
	packet.adds("/b_set");
	packet.maketags(numheads * 2 + 2);
	packet.addtag(',');
	packet.addtag('i');
	packet.addi(bufindex);
	
	while (msg.remain() >= 4)
	{	
		uint32 index = msg.geti();
		if (index >= maxIndex)
			return kSCErr_IndexOutOfRange;
		packet.addtag('i');
		packet.addtag('f');
		packet.addi(index);
		packet.addf(data[index]);
	}
	
	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}
	
	return kSCErr_None;
}

SCErr meth_b_getn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_b_getn(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{		
	sc_msg_iter msg(inSize, inData);
	int bufindex = msg.geti();
	SndBuf* buf = World_GetBuf(inWorld, bufindex);
	if (!buf) return kSCErr_Failed;
	
	float *data = buf->data;
	int32 maxIndex = buf->samples;
	
	// figure out how many tags to allocate   
	int numcontrols = 0;
	int numheads = msg.remain() >> 3;
	
	while (msg.remain()) {
		msg.geti(); // skip start
		int32 n = msg.geti();
		numcontrols += n;
	}
	
	scpacket packet;
	packet.adds("/b_setn");
	packet.maketags(numheads * 2 + numcontrols + 2);
	packet.addtag(',');

	// start over at beginning of message
	msg.init(inSize, inData);
	msg.geti(); // skip buf index

	packet.addtag('i');
	packet.addi(bufindex);
	
	while (msg.remain()) {
		int32 start = msg.geti();
		int32 n = msg.geti();
		int32 end = start+n-1;
		
		if (start < 0 || end >= maxIndex || start > end) 
			return kSCErr_IndexOutOfRange;
		
		packet.addtag('i');
		packet.addtag('i');
		packet.addi(start);
		packet.addi(n);

		for (int i=start; i<=end; ++i) {
			packet.addtag('f');
			packet.addf(data[i]);
		}
	}

	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}

	return kSCErr_None;
}


SCErr meth_s_get(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_s_get(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{
	sc_msg_iter msg(inSize, inData);

	Graph *graph = Msg_GetGraph(inWorld, msg);
	if (!graph) return kSCErr_NodeNotFound;
			
	int numheads = msg.remain() >> 2;

	scpacket packet;
	packet.adds("/n_set");
	packet.maketags(numheads * 2 + 1);
	packet.addtag(',');
	packet.addtag('i');
	packet.addi(graph->mNode.mID);
	
	while (msg.remain() >= 4) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int32 hash = Hash(name);
			float32 value = 0.f;
			Graph_GetControl(graph, hash, name, 0, value);
			packet.addtag('s');
			packet.addtag('f');
			packet.adds((char*)name);
			packet.addf(value);
		} else {
			int32 index = msg.geti();
			float32 value = 0.f;
			Graph_GetControl(graph, index, value);
			packet.addtag('i');
			packet.addtag('f');
			packet.addi(index);
			packet.addf(value);
		}
	}
	
	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}
	
	return kSCErr_None;
}

SCErr meth_s_getn(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_s_getn(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{		
	sc_msg_iter msg(inSize, inData);

	Graph *graph = Msg_GetGraph(inWorld, msg);
	if (!graph) return kSCErr_NodeNotFound;
	
	// figure out how many tags to allocate   
	int numcontrols = 0;
	int numheads = msg.remain() >> 3;
	
	while (msg.remain()) {
		msg.geti(); // skip start
		int32 n = msg.geti();
		numcontrols += n;
	}
	
	scpacket packet;
	packet.adds("/b_setn");
	packet.maketags(numheads * 2 + numcontrols + 2);
	packet.addtag(',');

	// start over at beginning of message
	msg.init(inSize, inData);
	msg.geti(); // skip buf index

	packet.addtag('i');
	packet.addi(graph->mNode.mID);
	
	while (msg.remain()) {
		if (msg.nextTag('i') == 's') {
			int32* name = msg.gets4();
			int32 hash = Hash(name);
			int32 n = msg.geti();
			packet.addtag('s');
			packet.addtag('i');
			packet.adds((char*)name);
			packet.addi(n);
			for (int i=0; i<n; ++i) {
				float32 value = 0.f;
				Graph_GetControl(graph, hash, name, i, value);
				packet.addtag('f');
				packet.addf(value);
			}
		} else {
			int32 index = msg.geti();
			int32 n = msg.geti();
			packet.addtag('i');
			packet.addtag('i');
			packet.addi(index);
			packet.addi(n);
			for (int i=0; i<n; ++i) {
				float32 value = 0.f;
				Graph_GetControl(graph, index+i, value);
				packet.addtag('f');
				packet.addf(value);
			}
		}
	}

	if (packet.size()) {
		CallSequencedCommand(SendReplyCmd, inWorld, packet.size(), packet.data(), inReply);
	}

	return kSCErr_None;
}



SCErr meth_s_noid(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_s_noid(World *inWorld, int inSize, char *inData, ReplyAddress* inReply)
{		
	sc_msg_iter msg(inSize, inData);
	while (msg.remain()) {

		Graph *graph = Msg_GetGraph(inWorld, msg);
		if (!graph) continue;
		
		Graph_RemoveID(inWorld, graph);
	}
	
	return kSCErr_None;
}


SCErr meth_notify(World *inWorld, int inSize, char *inData, ReplyAddress *inReply);
SCErr meth_notify(World *inWorld, int inSize, char *inData, ReplyAddress *inReply)
{
	CallSequencedCommand(NotifyCmd, inWorld, inSize, inData, inReply);
	return kSCErr_None;
}


#define NEW_COMMAND(name) NewCommand(#name, cmd_##name, meth_##name)

void initMiscCommands();
void initMiscCommands()
{

// nrt
	NEW_COMMAND(none);		
	NEW_COMMAND(notify);		
	NEW_COMMAND(status);	
	NEW_COMMAND(quit);		
	NEW_COMMAND(clearSched);		
		
	NEW_COMMAND(d_recv);		
	NEW_COMMAND(d_load);		
	NEW_COMMAND(d_loadDir);	
	NEW_COMMAND(d_freeAll);	

	NEW_COMMAND(s_new);	
			
	NEW_COMMAND(n_trace);
	NEW_COMMAND(n_free);		
	NEW_COMMAND(n_run);				

	NEW_COMMAND(u_cmd);	
	NEW_COMMAND(cmd);	
		
	//NEW_COMMAND(n_cmd);		
	NEW_COMMAND(n_map);		
	NEW_COMMAND(n_mapn);		
	NEW_COMMAND(n_set);		
	NEW_COMMAND(n_setn);		
	NEW_COMMAND(n_fill);		
	
	NEW_COMMAND(n_before);		
	NEW_COMMAND(n_after);		

	NEW_COMMAND(g_new);			
	NEW_COMMAND(g_head);		
	NEW_COMMAND(g_tail);		
	NEW_COMMAND(g_freeAll);		
	NEW_COMMAND(g_deepFree);		

	NEW_COMMAND(b_alloc);		
	NEW_COMMAND(b_allocRead);	
		
	NEW_COMMAND(b_read);		
	NEW_COMMAND(b_write);	
		
	NEW_COMMAND(b_free);		
	NEW_COMMAND(b_close);		
	
	NEW_COMMAND(b_zero);		
	NEW_COMMAND(b_set);		
	NEW_COMMAND(b_setn);		
	NEW_COMMAND(b_fill);			
	NEW_COMMAND(b_gen);			

	NEW_COMMAND(c_set);		
	NEW_COMMAND(c_setn);		
	NEW_COMMAND(c_fill);	
					
	NEW_COMMAND(dumpOSC);					

	NEW_COMMAND(c_get);		
	NEW_COMMAND(c_getn);
	NEW_COMMAND(b_get);		
	NEW_COMMAND(b_getn);
	NEW_COMMAND(s_get);		
	NEW_COMMAND(s_getn);
			
	NEW_COMMAND(n_query);		
	NEW_COMMAND(b_query);
			
	NEW_COMMAND(s_noid);		
}



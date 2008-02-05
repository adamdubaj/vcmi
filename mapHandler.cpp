#include "stdafx.h"
#include "mapHandler.h"
#include "hch\CSemiDefHandler.h"
#include "SDL_rotozoom.h"
#include "SDL_Extensions.h"
#include "CGameInfo.h"
#include "stdlib.h"
#include "hch\CLodHandler.h"
#include "hch\CDefObjInfoHandler.h"
#include <algorithm>
#include "CGameState.h"
#include "CLua.h"
#include "hch\CCastleHandler.h"
#include "hch\CHeroHandler.h"
#include "hch\CTownHandler.h"
#include <iomanip>
#include <sstream>
extern SDL_Surface * ekran;

class OCM_HLP
{
public:
	bool operator ()(const std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> & a, const std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> & b)
	{
		return (*a.first)<(*b.first);
	}
} ocmptwo ;
void alphaTransformDef(CGDefInfo * defInfo)
{	
	SDL_Surface * alphaTransSurf = SDL_CreateRGBSurface(SDL_SWSURFACE, 12, 12, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	for(int yy=0;yy<defInfo->handler->ourImages.size();yy++)
	{
		defInfo->handler->ourImages[yy].bitmap = CSDL_Ext::alphaTransform(defInfo->handler->ourImages[yy].bitmap);
		SDL_Surface * bufs = CSDL_Ext::secondAlphaTransform(defInfo->handler->ourImages[yy].bitmap, alphaTransSurf);
		SDL_FreeSurface(defInfo->handler->ourImages[yy].bitmap);
		defInfo->handler->ourImages[yy].bitmap = bufs;
		defInfo->handler->alphaTransformed = true;
	}
	SDL_FreeSurface(alphaTransSurf);
}
std::pair<int,int> CMapHandler::pickObject(CGObjectInstance *obj)
{
	switch(obj->ID)
	{
	case 65: //random artifact
		return std::pair<int,int>(5,(rand()%136)+7); //tylko sensowny zakres - na poczatku sa katapulty itp, na koncu specjalne i blanki
	case 66: //random treasure artifact
		return std::pair<int,int>(5,CGI->arth->treasures[rand()%CGI->arth->treasures.size()]->id);
	case 67: //random minor artifact
		return std::pair<int,int>(5,CGI->arth->minors[rand()%CGI->arth->minors.size()]->id);
	case 68: //random major artifact
		return std::pair<int,int>(5,CGI->arth->majors[rand()%CGI->arth->majors.size()]->id);
	case 69: //random relic artifact
		return std::pair<int,int>(5,CGI->arth->relics[rand()%CGI->arth->relics.size()]->id);
	case 70: //random hero
		return std::pair<int,int>(34,rand()%CGI->heroh->heroes.size());
	case 71: //random monster
		return std::pair<int,int>(54,rand()%(CGI->creh->creatures.size())); 
	case 72: //random monster lvl1
		return std::pair<int,int>(54,CGI->creh->levelCreatures[1][rand()%CGI->creh->levelCreatures[1].size()]->idNumber); 
	case 73: //random monster lvl2
		return std::pair<int,int>(54,CGI->creh->levelCreatures[2][rand()%CGI->creh->levelCreatures[2].size()]->idNumber);
	case 74: //random monster lvl3
		return std::pair<int,int>(54,CGI->creh->levelCreatures[3][rand()%CGI->creh->levelCreatures[3].size()]->idNumber);
	case 75: //random monster lvl4
		return std::pair<int,int>(54,CGI->creh->levelCreatures[4][rand()%CGI->creh->levelCreatures[4].size()]->idNumber);
	case 76: //random resource
		return std::pair<int,int>(79,rand()%7); //now it's OH3 style, use %8 for mithril 
	case 77: //random town
		return std::pair<int,int>(98,rand()%CGI->townh->towns.size()); 
	case 162: //random monster lvl5
		return std::pair<int,int>(54,CGI->creh->levelCreatures[5][rand()%CGI->creh->levelCreatures[5].size()]->idNumber);
	case 163: //random monster lvl6
		return std::pair<int,int>(54,CGI->creh->levelCreatures[6][rand()%CGI->creh->levelCreatures[6].size()]->idNumber);
	case 164: //random monster lvl7
		return std::pair<int,int>(54,CGI->creh->levelCreatures[7][rand()%CGI->creh->levelCreatures[7].size()]->idNumber); 
	case 216: //random dwelling
		{
			int faction = rand()%F_NUMBER;
			CCreGen2ObjInfo* info =(CCreGen2ObjInfo*)obj->info;
			if (info->asCastle)
			{
				for(int i=0;i<CGI->objh->objInstances.size();i++)
				{
					if(CGI->objh->objInstances[i]->ID==77 && dynamic_cast<CGTownInstance*>(CGI->objh->objInstances[i])->identifier == info->identifier)
					{
						randomizeObject(CGI->objh->objInstances[i]); //we have to randomize the castle first
						faction = CGI->objh->objInstances[i]->subID;
						break;
					}
					else if(CGI->objh->objInstances[i]->ID==98 && dynamic_cast<CGTownInstance*>(CGI->objh->objInstances[i])->identifier == info->identifier)
					{
						faction = CGI->objh->objInstances[i]->subID;
						break;
					}
				}
			}
			else
			{
				while((!(info->castles[0]&(1<<faction))))
				{
					if((faction>7) && (info->castles[1]&(1<<(faction-8))))
						break;
					faction = rand()%F_NUMBER;
				}
			}
			int level = ((info->maxLevel-info->minLevel) ? (rand()%(info->maxLevel-info->minLevel)+info->minLevel) : (info->minLevel));
			int cid = CGI->townh->towns[faction].basicCreatures[level];
			for(int i=0;i<CGI->objh->cregens.size();i++)
				if(CGI->objh->cregens[i]==cid)
					return std::pair<int,int>(17,i); 
			std::cout << "Cannot find a dwelling for creature "<<cid <<std::endl;
			return std::pair<int,int>(17,0); 
		}
	case 217:
		{
			int faction = rand()%F_NUMBER;
			CCreGenObjInfo* info =(CCreGenObjInfo*)obj->info;
			if (info->asCastle)
			{
				for(int i=0;i<CGI->objh->objInstances.size();i++)
				{
					if(CGI->objh->objInstances[i]->ID==77 && dynamic_cast<CGTownInstance*>(CGI->objh->objInstances[i])->identifier == info->identifier)
					{
						randomizeObject(CGI->objh->objInstances[i]); //we have to randomize the castle first
						faction = CGI->objh->objInstances[i]->subID;
						break;
					}
					else if(CGI->objh->objInstances[i]->ID==98 && dynamic_cast<CGTownInstance*>(CGI->objh->objInstances[i])->identifier == info->identifier)
					{
						faction = CGI->objh->objInstances[i]->subID;
						break;
					}
				}
			}
			else
			{
				while((!(info->castles[0]&(1<<faction))))
				{
					if((faction>7) && (info->castles[1]&(1<<(faction-8))))
						break;
					faction = rand()%F_NUMBER;
				}
			}
			int cid = CGI->townh->towns[faction].basicCreatures[obj->subID];
			for(int i=0;i<CGI->objh->cregens.size();i++)
				if(CGI->objh->cregens[i]==cid)
					return std::pair<int,int>(17,i); 
			std::cout << "Cannot find a dwelling for creature "<<cid <<std::endl;
			return std::pair<int,int>(17,0); 
		}
	case 218:
		{
			CCreGen3ObjInfo* info =(CCreGen3ObjInfo*)obj->info;
			int level = ((info->maxLevel-info->minLevel) ? (rand()%(info->maxLevel-info->minLevel)+info->minLevel) : (info->minLevel));
			int cid = CGI->townh->towns[obj->subID].basicCreatures[level];
			for(int i=0;i<CGI->objh->cregens.size();i++)
				if(CGI->objh->cregens[i]==cid)
					return std::pair<int,int>(17,i); 
			std::cout << "Cannot find a dwelling for creature "<<cid <<std::endl;
			return std::pair<int,int>(17,0); 
		}
	}
	return std::pair<int,int>(-1,-1);
}
void CMapHandler::randomizeObject(CGObjectInstance *cur)
{		
	std::pair<int,int> ran = pickObject(cur);
	if(ran.first<0 || ran.second<0) //this is not a random object, or we couldn't find anything
		return;
	else if(ran.first==34)//special code for hero
	{
		CGHeroInstance *h = dynamic_cast<CGHeroInstance *>(cur);
		if(!h) {std::cout<<"Wrong random hero at "<<cur->pos<<std::endl; return;}
		cur->ID = ran.first;
		cur->subID = ran.second;
		h->type = CGI->heroh->heroes[ran.second];
		CGI->heroh->heroInstances.push_back(h);
		CGI->objh->objInstances.erase(std::find(CGI->objh->objInstances.begin(),CGI->objh->objInstances.end(),h));
		return; //TODO: maybe we should do something with definfo?
	}
	else if(ran.first==98)//special code for town
	{
		CGTownInstance *t = dynamic_cast<CGTownInstance*>(cur);
		if(!t) {std::cout<<"Wrong random town at "<<cur->pos<<std::endl; return;}
		cur->ID = ran.first;
		cur->subID = ran.second;
		t->town = &CGI->townh->towns[ran.second];
		if(t->hasCapitol())
			t->defInfo = capitols[t->subID];
		else if(t->hasFort())
			t->defInfo = CGI->dobjinfo->castles[t->subID];
		else
			t->defInfo = villages[t->subID]; 
		if(!t->defInfo->handler)
		{
			t->defInfo->handler = CGI->spriteh->giveDef(t->defInfo->name);
			alphaTransformDef(t->defInfo);
		}
		CGI->townh->townInstances.push_back(t);
		return;
	}
	//we have to replace normal random object
	cur->ID = ran.first;
	cur->subID = ran.second;
	cur->defInfo = CGI->dobjinfo->gobjs[ran.first][ran.second];
	if(!cur->defInfo){std::cout<<"Missing def declaration for "<<cur->ID<<" "<<cur->subID<<std::endl;return;}
	if(!cur->defInfo->handler) //if we have to load def
	{
		cur->defInfo->handler = CGI->spriteh->giveDef(cur->defInfo->name);
		alphaTransformDef(cur->defInfo);
	}

}
void CMapHandler::randomizeObjects()
{
	CGObjectInstance * cur;
	for(int no=0; no<CGI->objh->objInstances.size(); ++no)
	{
		randomizeObject(CGI->objh->objInstances[no]);
	}
}
void CMapHandler::prepareFOWDefs()
{
	fullHide = CGameInfo::mainObj->spriteh->giveDef("TSHRC.DEF");
	partialHide = CGameInfo::mainObj->spriteh->giveDef("TSHRE.DEF");

	//adding necessary rotations
	Cimage nw = partialHide->ourImages[22]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[15]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[2]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[13]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[12]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[16]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[18]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[17]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[20]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[19]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[7]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[24]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[26]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[25]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[30]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[32]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[27]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	nw = partialHide->ourImages[28]; nw.bitmap = CSDL_Ext::rotate01(nw.bitmap);
	partialHide->ourImages.push_back(nw);
	//necessaary rotations added

	for(int i=0; i<partialHide->ourImages.size(); ++i)
	{
		CSDL_Ext::fullAlphaTransform(partialHide->ourImages[i].bitmap);
	}
	//visibility.resize(reader->map.width+2*Woff);
	//for(int gg=0; gg<reader->map.width+2*Woff; ++gg)
	//{
	//	visibility[gg].resize(reader->map.height+2*Hoff);
	//	for(int jj=0; jj<reader->map.height+2*Hoff; ++jj)
	//		visibility[gg][jj] = true;
	//}

	visibility.resize(CGI->ac->map.width, Woff);
	for (int i=0-Woff;i<visibility.size()-Woff;i++)
	{
		visibility[i].resize(CGI->ac->map.height,Hoff);
	}
	for (int i=0-Woff; i<visibility.size()-Woff; ++i)
	{
		for (int j=0-Hoff; j<CGI->ac->map.height+Hoff; ++j)
		{
			visibility[i][j].resize(CGI->ac->map.twoLevel+1,0);
			for(int k=0; k<CGI->ac->map.twoLevel+1; ++k)
				visibility[i][j][k]=true;
		}
	}

	hideBitmap.resize(CGI->ac->map.width, Woff);
	for (int i=0-Woff;i<visibility.size()-Woff;i++)
	{
		hideBitmap[i].resize(CGI->ac->map.height,Hoff);
	}
	for (int i=0-Woff; i<hideBitmap.size()-Woff; ++i)
	{
		for (int j=0-Hoff; j<CGI->ac->map.height+Hoff; ++j)
		{
			hideBitmap[i][j].resize(CGI->ac->map.twoLevel+1,0);
			for(int k=0; k<CGI->ac->map.twoLevel+1; ++k)
				hideBitmap[i][j][k] = rand()%fullHide->ourImages.size();
		}
	}

	//visibility[6][7][1] = false;
	//visibility[7][7][1] = false;
	//visibility[6][8][1] = false;
	//visibility[6][6][1] = false;
	//visibility[5][8][1] = false;
	//visibility[7][6][1] = false;
	//visibility[6][9][1] = false;
}

void CMapHandler::roadsRiverTerrainInit()
{
	//initializing road's and river's DefHandlers

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		int rmask = 0xff000000;
		int gmask = 0x00ff0000;
		int bmask = 0x0000ff00;
		int amask = 0x000000ff;
	#else
		int rmask = 0x000000ff;
		int gmask = 0x0000ff00;
		int bmask = 0x00ff0000;
		int amask = 0xff000000;
	#endif

	SDL_Surface * su = SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
		rmask, gmask, bmask, amask);

	roadDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("dirtrd.def"));
	roadDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("gravrd.def"));
	roadDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("cobbrd.def"));
	staticRiverDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("clrrvr.def"));
	staticRiverDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("icyrvr.def"));
	staticRiverDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("mudrvr.def"));
	staticRiverDefs.push_back(CGameInfo::mainObj->spriteh->giveDef("lavrvr.def"));

	//roadBitmaps = new SDL_Surface** [reader->map.width+2*Woff];
	//for (int ii=0;ii<reader->map.width+2*Woff;ii++)
	//	roadBitmaps[ii] = new SDL_Surface*[reader->map.height+2*Hoff]; // allocate memory
	sizes.x = CGI->ac->map.width;
	sizes.y = CGI->ac->map.height;
	sizes.z = CGI->ac->map.twoLevel+1;
	ttiles.resize(CGI->ac->map.width,Woff);
	for (int i=0-Woff;i<ttiles.size()-Woff;i++)
	{
		ttiles[i].resize(CGI->ac->map.height,Hoff);
	}
	for (int i=0-Woff;i<ttiles.size()-Woff;i++)
	{
		for (int j=0-Hoff;j<CGI->ac->map.height+Hoff;j++)
			ttiles[i][j].resize(CGI->ac->map.twoLevel+1,0);
	}



	for (int i=0; i<reader->map.width; i++) //jest po szerokości
	{
		for (int j=0; j<reader->map.height;j++) //po wysokości
		{
			for (int k=0; k<=reader->map.twoLevel; ++k)
			{
				TerrainTile** pomm = reader->map.terrain; ;
				if (k==0)
					pomm = reader->map.terrain;
				else
					pomm = reader->map.undergroungTerrain;
				if(pomm[i][j].malle)
				{
					int cDir;
					bool rotV, rotH;
					if(k==0)
					{
						int roadpom = reader->map.terrain[i][j].malle-1,
							impom = reader->map.terrain[i][j].roadDir;
						SDL_Surface *pom1 = roadDefs[roadpom]->ourImages[impom].bitmap;
						ttiles[i][j][k].roadbitmap.push_back(pom1);
						cDir = reader->map.terrain[i][j].roadDir;

						rotH = (reader->map.terrain[i][j].siodmyTajemniczyBajt >> 5) & 1;
						rotV = (reader->map.terrain[i][j].siodmyTajemniczyBajt >> 4) & 1;
					}
					else
					{
						int pom111 = reader->map.undergroungTerrain[i][j].malle-1,
							pom777 = reader->map.undergroungTerrain[i][j].roadDir;
						SDL_Surface *pom1 = roadDefs[pom111]->ourImages[pom777].bitmap;
						ttiles[i][j][k].roadbitmap.push_back(pom1);
						cDir = reader->map.undergroungTerrain[i][j].roadDir;

						rotH = (reader->map.undergroungTerrain[i][j].siodmyTajemniczyBajt >> 5) & 1;
						rotV = (reader->map.undergroungTerrain[i][j].siodmyTajemniczyBajt >> 4) & 1;
					}
					if(rotH)
					{
						ttiles[i][j][k].roadbitmap[0] = CSDL_Ext::hFlip(ttiles[i][j][k].roadbitmap[0]);
					}
					if(rotV)
					{
						ttiles[i][j][k].roadbitmap[0] = CSDL_Ext::rotate01(ttiles[i][j][k].roadbitmap[0]);
					}
					if(rotH || rotV)
					{
						ttiles[i][j][k].roadbitmap[0] = CSDL_Ext::alphaTransform(ttiles[i][j][k].roadbitmap[0]);
						SDL_Surface * buf = CSDL_Ext::secondAlphaTransform(ttiles[i][j][k].roadbitmap[0], su);
						SDL_FreeSurface(ttiles[i][j][k].roadbitmap[0]);
						ttiles[i][j][k].roadbitmap[0] = buf;
					}
				}
			}
		}
	}

	//initializing simple values
	for (int i=0; i<CGI->ac->map.width; i++) //jest po szerokości
	{
		for (int j=0; j<CGI->ac->map.height;j++) //po wysokości
		{
			for(int k=0; k<ttiles[0][0].size(); ++k)
			{
				ttiles[i][j][k].pos = int3(i, j, k);
				ttiles[i][j][k].blocked = false;
				ttiles[i][j][k].visitable = false;
				if(i<0 || j<0 || i>=CGI->ac->map.width || j>=CGI->ac->map.height)
				{
					ttiles[i][j][k].blocked = true;
					continue;
				}
				ttiles[i][j][k].terType = (k==0 ? CGI->ac->map.terrain[i][j].tertype : CGI->ac->map.undergroungTerrain[i][j].tertype);
				ttiles[i][j][k].malle = (k==0 ? CGI->ac->map.terrain[i][j].malle : CGI->ac->map.undergroungTerrain[i][j].malle);
				ttiles[i][j][k].nuine = (k==0 ? CGI->ac->map.terrain[i][j].nuine : CGI->ac->map.undergroungTerrain[i][j].nuine);
				ttiles[i][j][k].rivdir = (k==0 ? CGI->ac->map.terrain[i][j].rivDir : CGI->ac->map.undergroungTerrain[i][j].rivDir);
				ttiles[i][j][k].roaddir = (k==0 ? CGI->ac->map.terrain[i][j].roadDir : CGI->ac->map.undergroungTerrain[i][j].roadDir);

			}
		}
	}
	//simple values initialized

	for (int i=0; i<reader->map.width; i++) //jest po szerokości
	{
		for (int j=0; j<reader->map.height;j++) //po wysokości
		{
			for(int k=0; k<=reader->map.twoLevel; ++k)
			{
				TerrainTile** pomm = reader->map.terrain;
				if(k==0)
				{
					pomm = reader->map.terrain;
				}
				else
				{
					pomm = reader->map.undergroungTerrain;
				}
				if(pomm[i][j].nuine)
				{
					int cDir;
					bool rotH, rotV;
					if(k==0)
					{
						ttiles[i][j][k].rivbitmap.push_back(staticRiverDefs[reader->map.terrain[i][j].nuine-1]->ourImages[reader->map.terrain[i][j].rivDir].bitmap);
						cDir = reader->map.terrain[i][j].rivDir;
						rotH = (reader->map.terrain[i][j].siodmyTajemniczyBajt >> 3) & 1;
						rotV = (reader->map.terrain[i][j].siodmyTajemniczyBajt >> 2) & 1;
					}
					else
					{
						ttiles[i][j][k].rivbitmap.push_back(staticRiverDefs[reader->map.undergroungTerrain[i][j].nuine-1]->ourImages[reader->map.undergroungTerrain[i][j].rivDir].bitmap);
						cDir = reader->map.undergroungTerrain[i][j].rivDir;
						rotH = (reader->map.undergroungTerrain[i][j].siodmyTajemniczyBajt >> 3) & 1;
						rotV = (reader->map.undergroungTerrain[i][j].siodmyTajemniczyBajt >> 2) & 1;
					}
					if(rotH)
					{
						ttiles[i][j][k].rivbitmap[0] = CSDL_Ext::hFlip(ttiles[i][j][k].rivbitmap[0]);
					}
					if(rotV)
					{
						ttiles[i][j][k].rivbitmap[0] = CSDL_Ext::rotate01(ttiles[i][j][k].rivbitmap[0]);
					}
					if(rotH || rotV)
					{
						ttiles[i][j][k].rivbitmap[0] = CSDL_Ext::alphaTransform(ttiles[i][j][k].rivbitmap[0]);
						SDL_Surface * buf = CSDL_Ext::secondAlphaTransform(ttiles[i][j][k].rivbitmap[0], su);
						SDL_FreeSurface(ttiles[i][j][k].rivbitmap[0]);
						ttiles[i][j][k].rivbitmap[0] = buf;
					}
				}
			}
		}
	}

	SDL_FreeSurface(su);
}
void CMapHandler::borderAndTerrainBitmapInit()
{
	//terrainBitmap = new SDL_Surface **[reader->map.width+2*Woff];
	//for (int ii=0;ii<reader->map.width+2*Woff;ii++)
	//	terrainBitmap[ii] = new SDL_Surface*[reader->map.height+2*Hoff]; // allocate memory

	CDefHandler * bord = CGameInfo::mainObj->spriteh->giveDef("EDG.DEF");
	for (int i=0-Woff; i<reader->map.width+Woff; i++) //jest po szerokości
	{
		for (int j=0-Hoff; j<reader->map.height+Hoff;j++) //po wysokości
		{
			for(int k=0; k<=reader->map.twoLevel; ++k)
			{
				if(i < 0 || i > (reader->map.width-1) || j < 0  || j > (reader->map.height-1))
				{
					if(i==-1 && j==-1)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[16].bitmap);
						continue;
					}
					else if(i==-1 && j==(reader->map.height))
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[19].bitmap);
						continue;
					}
					else if(i==(reader->map.width) && j==-1)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[17].bitmap);
						continue;
					}
					else if(i==(reader->map.width) && j==(reader->map.height))
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[18].bitmap);
						continue;
					}
					else if(j == -1 && i > -1 && i < reader->map.height)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[22+rand()%2].bitmap);
						continue;
					}
					else if(i == -1 && j > -1 && j < reader->map.height)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[33+rand()%2].bitmap);
						continue;
					}
					else if(j == reader->map.height && i >-1 && i < reader->map.width)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[29+rand()%2].bitmap);
						continue;
					}
					else if(i == reader->map.width && j > -1 && j < reader->map.height)
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[25+rand()%2].bitmap);
						continue;
					}
					else
					{
						ttiles[i][j][k].terbitmap.push_back(bord->ourImages[rand()%16].bitmap);
						continue;
					}
				}
				//TerrainTile zz = reader->map.terrain[i-Woff][j-Hoff];
				std::string name;
				if (k>0)
					name = CSemiDefHandler::nameFromType(reader->map.undergroungTerrain[i][j].tertype);
				else
					name = CSemiDefHandler::nameFromType(reader->map.terrain[i][j].tertype);
				for (unsigned int m=0; m<reader->defs.size(); m++)
				{
					try
					{
						if (reader->defs[m]->defName != name)
							continue;
						else
						{
							int ktora;
							if (k==0)
								ktora = reader->map.terrain[i][j].terview;
							else
								ktora = reader->map.undergroungTerrain[i][j].terview;
							ttiles[i][j][k].terbitmap.push_back(reader->defs[m]->ourImages[ktora].bitmap);
							int zz;
							if (k==0)
								zz = (reader->map.terrain[i][j].siodmyTajemniczyBajt)%4;
							else
								zz = (reader->map.undergroungTerrain[i][j].siodmyTajemniczyBajt)%4;
							switch (zz)
							{
							case 1:
								{
									ttiles[i][j][k].terbitmap[0] = CSDL_Ext::rotate01(ttiles[i][j][k].terbitmap[0]);
									break;
								}
							case 2:
								{
									ttiles[i][j][k].terbitmap[0] = CSDL_Ext::hFlip(ttiles[i][j][k].terbitmap[0]);
									break;
								}
							case 3:
								{
									ttiles[i][j][k].terbitmap[0] = CSDL_Ext::rotate03(ttiles[i][j][k].terbitmap[0]);
									break;
								}
							}

							break;
						}
					}
					catch (...)
					{
						continue;
					}
				}
			}
		}
	}
}
void CMapHandler::initObjectRects()
{
	//initializing objects / rects
	for(int f=0; f<CGI->objh->objInstances.size(); ++f)
	{
		/*CGI->objh->objInstances[f]->pos.x+=1;
		CGI->objh->objInstances[f]->pos.y+=1;*/
		if(!CGI->objh->objInstances[f]->defInfo)
		{
			continue;
		}
		CDefHandler * curd = CGI->objh->objInstances[f]->defInfo->handler;
		for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
		{
			for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
			{
				SDL_Rect cr;
				cr.w = 32;
				cr.h = 32;
				cr.x = fx*32;
				cr.y = fy*32;
				std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> toAdd = std::make_pair(CGI->objh->objInstances[f], std::make_pair(cr, std::vector<std::list<int3>>()));
				///initializing places that will be coloured by blitting (flag colour / player colour positions)
				if(toAdd.first->defInfo->isVisitable())
				{
					toAdd.second.second.resize(toAdd.first->defInfo->handler->ourImages.size());
					for(int no = 0; no<toAdd.first->defInfo->handler->ourImages.size(); ++no)
					{
						bool breakNow = true;
						for(int dx=0; dx<32; ++dx)
						{
							for(int dy=0; dy<32; ++dy)
							{
								SDL_Surface * curs = toAdd.first->defInfo->handler->ourImages[no].bitmap;
								Uint32* point = (Uint32*)( (Uint8*)curs->pixels + curs->pitch * (fy*32+dy) + curs->format->BytesPerPixel*(fx*32+dx));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*point, curs->format, &r, &g, &b, &a);
								if(r==255 && g==255 && b==0)
								{
									toAdd.second.second[no].push_back(int3((fx*32+dx), (fy*32+dy), 0));
									breakNow = false;
								}
							}
						}
						if(breakNow)
							break;
					}
				}
				if((CGI->objh->objInstances[f]->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)>=0 && (CGI->objh->objInstances[f]->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)<ttiles.size()-Woff && (CGI->objh->objInstances[f]->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)>=0 && (CGI->objh->objInstances[f]->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)<ttiles[0].size()-Hoff)
				{
					TerrainTile2 & curt =
						ttiles
						[CGI->objh->objInstances[f]->pos.x + fx - curd->ourImages[0].bitmap->w/32]
					[CGI->objh->objInstances[f]->pos.y + fy - curd->ourImages[0].bitmap->h/32]
					[CGI->objh->objInstances[f]->pos.z];


					ttiles[CGI->objh->objInstances[f]->pos.x + fx - curd->ourImages[0].bitmap->w/32+1][CGI->objh->objInstances[f]->pos.y + fy - curd->ourImages[0].bitmap->h/32+1][CGI->objh->objInstances[f]->pos.z].objects.push_back(toAdd);
				}

			} // for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
		} //for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
	} // for(int f=0; f<CGI->objh->objInstances.size(); ++f)
	for(int ix=0; ix<ttiles.size()-Woff; ++ix)
	{
		for(int iy=0; iy<ttiles[0].size()-Hoff; ++iy)
		{
			for(int iz=0; iz<ttiles[0][0].size(); ++iz)
			{
				stable_sort(ttiles[ix][iy][iz].objects.begin(), ttiles[ix][iy][iz].objects.end(), ocmptwo);
			}
		}
	}
}
void CMapHandler::calculateBlockedPos()
{
	for(int f=0; f<CGI->objh->objInstances.size(); ++f) //calculationg blocked / visitable positions
	{
		if(!CGI->objh->objInstances[f]->defInfo)
			continue;
		CDefHandler * curd = CGI->objh->objInstances[f]->defInfo->handler;
		for(int fx=0; fx<8; ++fx)
		{
			for(int fy=0; fy<6; ++fy)
			{
				int xVal = CGI->objh->objInstances[f]->pos.x + fx - 7;
				int yVal = CGI->objh->objInstances[f]->pos.y + fy - 5;
				int zVal = CGI->objh->objInstances[f]->pos.z;
				if(xVal>=0 && xVal<ttiles.size()-Woff && yVal>=0 && yVal<ttiles[0].size()-Hoff)
				{
					TerrainTile2 & curt = ttiles[xVal][yVal][zVal];
					if(((CGI->objh->objInstances[f]->defInfo->visitMap[fy] >> (7 - fx)) & 1))
						curt.visitable = true;
					if(!((CGI->objh->objInstances[f]->defInfo->blockMap[fy] >> (7 - fx)) & 1))
						curt.blocked = true;
				}
			}
		}
	}
}
void CMapHandler::init()
{
	std::ifstream ifs("config/townsDefs.txt");
	int ccc;
	ifs>>ccc;
	for(int i=0;i<ccc*2;i++)
	{
		CGDefInfo * n = new CGDefInfo(*CGI->dobjinfo->castles[i%ccc]);
		ifs >> n->name;
		if (!(n->handler = CGI->spriteh->giveDef(n->name)))
			std::cout << "Cannot open "<<n->name<<std::endl;
		if(i<ccc)
			villages[i]=n;
		else
			capitols[i%ccc]=n;
		alphaTransformDef(n);
	} 

	timeHandler th;
	th.getDif();
	randomizeObjects();//randomizing objects on map
	std::cout<<"\tRandomizing objects: "<<th.getDif()<<std::endl;

	for(int h=0; h<reader->map.defy.size(); ++h) //initializing loaded def handler's info
	{
		//std::string hlp = reader->map.defy[h]->name;
		//std::transform(hlp.begin(), hlp.end(), hlp.begin(), (int(*)(int))toupper);
		CGI->mh->loadedDefs.insert(std::make_pair(reader->map.defy[h]->name, reader->map.defy[h]->handler));
	}
	std::cout<<"\tCollecting loaded def's handlers: "<<th.getDif()<<std::endl;

	prepareFOWDefs();
	roadsRiverTerrainInit();	//road's and river's DefHandlers; and simple values initialization
	borderAndTerrainBitmapInit();
	std::cout<<"\tPreparing FoW, roads, rivers,borders: "<<th.getDif()<<std::endl;

	initObjectRects();
	std::cout<<"\tMaking object rects: "<<th.getDif()<<std::endl;

	calculateBlockedPos();
	std::cout<<"\tCalculating blockmap: "<<th.getDif()<<std::endl;
}

SDL_Surface * CMapHandler::terrainRect(int x, int y, int dx, int dy, int level, unsigned char anim, PseudoV< PseudoV< PseudoV<unsigned char> > > & visibilityMap, bool otherHeroAnim, unsigned char heroAnim)
{
	if(!otherHeroAnim)
		heroAnim = anim; //the same, as it should be
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif
	SDL_Surface * su = SDL_CreateRGBSurface(SDL_SWSURFACE, dx*32, dy*32, 32,
                                   rmask, gmask, bmask, amask);
	if (((dx+x)>((reader->map.width+Woff)) || (dy+y)>((reader->map.height+Hoff))) || ((x<-Woff)||(y<-Hoff) ) )
		throw new std::string("terrainRect: out of range");
	////printing terrain
	for (int bx=0; bx<dx; bx++)
	{
		for (int by=0; by<dy; by++)
		{
			SDL_Rect sr;
			sr.y=by*32;
			sr.x=bx*32;
			sr.h=sr.w=32;
			SDL_BlitSurface(ttiles[x+bx][y+by][level].terbitmap[anim%ttiles[x+bx][y+by][level].terbitmap.size()],NULL,su,&sr);
		}
	}
	////terrain printed
	////printing rivers
	for (int bx=0; bx<dx; bx++)
	{
		for (int by=0; by<dy; by++)
		{
			SDL_Rect sr;
			sr.y=by*32;
			sr.x=bx*32;
			sr.h=sr.w=32;
			if(ttiles[x+bx][y+by][level].rivbitmap.size())
				SDL_BlitSurface(ttiles[x+bx][y+by][level].rivbitmap[anim%ttiles[x+bx][y+by][level].rivbitmap.size()],NULL,su,&sr);
		}
	}
	////rivers printed
	////printing roads
	for (int bx=0; bx<dx; bx++)
	{
		for (int by=-1; by<dy; by++)
		{
			if(y+by<=-4)
				continue;
			SDL_Rect sr;
			sr.y=by*32+16;
			sr.x=bx*32;
			sr.h=sr.w=32;
			if(ttiles[x+bx][y+by][level].roadbitmap.size())
				SDL_BlitSurface(ttiles[x+bx][y+by][level].roadbitmap[anim%ttiles[x+bx][y+by][level].roadbitmap.size()],NULL,su,&sr);
		}
	}
	////roads printed
	////printing objects

	for (int bx=0; bx<dx; bx++)
	{
		for (int by=0; by<dy; by++)
		{
			for(int h=0; h<ttiles[x+bx][y+by][level].objects.size(); ++h)
			{
				SDL_Rect sr;
				sr.w = 32;
				sr.h = 32;
				sr.x = (bx)*32;
				sr.y = (by)*32;

				SDL_Rect pp = ttiles[x+bx][y+by][level].objects[h].second.first;
				CGHeroInstance * themp = (dynamic_cast<CGHeroInstance*>(ttiles[x+bx][y+by][level].objects[h].first));
				if(themp && themp->moveDir && !themp->isStanding && themp->ID!=62) //last condition - this is not prison
				{
					int imgVal = 8;
					SDL_Surface * tb;

					if(((CHeroObjInfo*)themp->info)->myInstance->type==NULL)
						continue;
					std::vector<Cimage> & iv = ((CHeroObjInfo*)themp->info)->myInstance->type->heroClass->moveAnim->ourImages;
					switch(themp->moveDir)
					{
					case 1:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==10)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 2:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==5)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);	
							break;
						}
					case 3:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==6)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 4:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==7)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 5:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==8)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 6: //ok
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==9)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 7:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==12)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					case 8:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==11)
								{
									tb = iv[gg+heroAnim%imgVal].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							pp.y+=imgVal*2-32;
							sr.y-=16;
							SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[gg+heroAnim%imgVal+35].bitmap, &pp, su, &sr);
							break;
						}
					}
				}
				else if(themp && themp->moveDir && themp->isStanding && themp->ID!=62) //last condition - this is not prison
				{
					int imgVal = 8;
					SDL_Surface * tb;

					if(((CHeroObjInfo*)themp->info)->myInstance->type==NULL)
						continue;
					std::vector<Cimage> & iv = ((CHeroObjInfo*)themp->info)->myInstance->type->heroClass->moveAnim->ourImages;
					switch(themp->moveDir)
					{
					case 1:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==13)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[13*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 2:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==0)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[heroAnim%imgVal].bitmap, NULL, su, &bufr);	
								themp->flagPrinted = true;
							}
							break;
						}
					case 3:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==1)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 4:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==2)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[2*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 5:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==3)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[3*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 6:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==4)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[4*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 7:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==15)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[15*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					case 8:
						{
							int gg;
							for(gg=0; gg<iv.size(); ++gg)
							{
								if(iv[gg].groupNumber==14)
								{
									tb = iv[gg].bitmap;
									break;
								}
							}
							SDL_BlitSurface(tb,&pp,su,&sr);
							if(themp->pos.x==x+bx && themp->pos.y==y+by)
							{
								SDL_Rect bufr = sr;
								bufr.x-=2*32;
								bufr.y-=1*32;
								SDL_BlitSurface(CGI->heroh->flags4[themp->getOwner()]->ourImages[14*8+heroAnim%imgVal].bitmap, NULL, su, &bufr);
								themp->flagPrinted = true;
							}
							break;
						}
					}
				}
				else
				{
					int imgVal = ttiles[x+bx][y+by][level].objects[h].first->defInfo->handler->ourImages.size();
					SDL_BlitSurface(ttiles[x+bx][y+by][level].objects[h].first->defInfo->handler->ourImages[anim%imgVal].bitmap,&pp,su,&sr);
				}
				//printing appropriate flag colour
				if(ttiles[x+bx][y+by][level].objects[h].second.second.size())
				{
					std::list<int3> & curl = ttiles[x+bx][y+by][level].objects[h].second.second[anim%ttiles[x+bx][y+by][level].objects[h].second.second.size()];
					for(std::list<int3>::iterator g=curl.begin(); g!=curl.end(); ++g)
					{
						SDL_Color ourC;
						int own = ttiles[x+bx][y+by][level].objects[h].first->getOwner();
						if(ttiles[x+bx][y+by][level].objects[h].first->getOwner()!=255 && ttiles[x+bx][y+by][level].objects[h].first->getOwner()!=254)
							ourC = CGI->playerColors[ttiles[x+bx][y+by][level].objects[h].first->getOwner()];
						else if(ttiles[x+bx][y+by][level].objects[h].first->getOwner()==255)
							ourC = CGI->neutralColor;
						else continue;
						CSDL_Ext::SDL_PutPixelWithoutRefresh(su, bx*32 + g->x%32 , by*32 + g->y%32, ourC.r , ourC.g, ourC.b, 0);
					}
				}
			}
		}
	}

	///enabling flags



	//nie zauwazylem aby ustawianie tego cokolwiek zmienialo w wyswietlaniu, wiec komentuje (do dzialania wymaga jeszcze odkomentowania przyjazni w statcie)

	/*for(std::map<int, PlayerState>::iterator k=CGI->state->players.begin(); k!=CGI->state->players.end(); ++k)
	{
		for (int l = 0; l<k->second.heroes.size(); l++)
			k->second.heroes[l]->flagPrinted = false;
	}
	for(int qq=0; qq<CGI->heroh->heroInstances.size(); ++qq)
	{
		CGI->heroh->heroInstances[qq]->flagPrinted = false;
	}*/

	///flags enabled

	////objects printed, printing shadow
	for (int bx=0; bx<dx; bx++)
	{
		for (int by=0; by<dy; by++)
		{
			SDL_Rect sr;
			sr.y=by*32;
			sr.x=bx*32;
			sr.h=sr.w=32;
			if (!level)
			{
				
				//if( bx+x>-1 && by+y>-1 && bx+x<visibilityMap.size()-(-1) && by+y<visibilityMap[0].size()-(-1) && !visibilityMap[bx+x][by+y][0])
				if(bx+x>=0 && by+y>=0 && bx+x<CGI->mh->reader->map.width && by+y<CGI->mh->reader->map.height && !visibilityMap[bx+x][by+y][0])
				{
					SDL_Surface * hide = getVisBitmap(bx+x, by+y, visibilityMap, 0);
					//SDL_Surface * hide2 = CSDL_Ext::secondAlphaTransform(hide, su);
					SDL_BlitSurface(hide, NULL, su, &sr);
					//SDL_FreeSurface(hide2);
				}
			}
			else
			{
				//if( bx+x>-1 && by+y>-1 && bx+x<visibilityMap.size()-(-1) && by+y<visibilityMap[0].size()-(-1) && !visibilityMap[bx+x][by+y][1])
				if(bx+x>=0 && by+y>=0 && bx+x<CGI->mh->reader->map.width && by+y<CGI->mh->reader->map.height && !visibilityMap[bx+x][by+y][1])
				{
					SDL_Surface * hide = getVisBitmap(bx+x, by+y, visibilityMap, 1);
					//SDL_Surface * hide2 = CSDL_Ext::secondAlphaTransform(hide, su);
					SDL_BlitSurface(hide, NULL, su, &sr);
					//SDL_FreeSurface(hide2);
				}
			}
		}
	}
	////shadow printed
	//printing borders
	for (int bx=0; bx<dx; bx++)
	{
		for (int by=0; by<dy; by++)
		{
			if(bx+x<0 || by+y<0 || bx+x>reader->map.width+(-1) || by+y>reader->map.height+(-1))
			{
				SDL_Rect sr;
				sr.y=by*32;
				sr.x=bx*32;
				sr.h=sr.w=32;

				SDL_BlitSurface(ttiles[x+bx][y+by][level].terbitmap[anim%ttiles[x+bx][y+by][level].terbitmap.size()],NULL,su,&sr);
			}
			else 
			{
				if(MARK_BLOCKED_POSITIONS &&  ttiles[x+bx][y+by][level].blocked) //temporary hiding blocked positions
				{
					SDL_Rect sr;
					sr.y=by*32;
					sr.x=bx*32;
					sr.h=sr.w=32;

					SDL_Surface * ns =  SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
									   rmask, gmask, bmask, amask);
					for(int f=0; f<ns->w*ns->h*4; ++f)
					{
						*((unsigned char*)(ns->pixels) + f) = 128;
					}

					SDL_BlitSurface(ns,NULL,su,&sr);

					SDL_FreeSurface(ns);
				}
				if(MARK_VISITABLE_POSITIONS &&  ttiles[x+bx][y+by][level].visitable) //temporary hiding visitable positions
				{
					SDL_Rect sr;
					sr.y=by*32;
					sr.x=bx*32;
					sr.h=sr.w=32;

					SDL_Surface * ns =  SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32,
									   rmask, gmask, bmask, amask);
					for(int f=0; f<ns->w*ns->h*4; ++f)
					{
						*((unsigned char*)(ns->pixels) + f) = 128;
					}

					SDL_BlitSurface(ns,NULL,su,&sr);

					SDL_FreeSurface(ns);
				}
			}
		}
	}
	CSDL_Ext::update(su);
	//borders printed
	return su;
}

SDL_Surface * CMapHandler::terrBitmap(int x, int y)
{
	return ttiles[x+Woff][y+Hoff][0].terbitmap[0];
}

SDL_Surface * CMapHandler::undTerrBitmap(int x, int y)
{
	return ttiles[x+Woff][y+Hoff][0].terbitmap[1];
}

SDL_Surface * CMapHandler::getVisBitmap(int x, int y, PseudoV< PseudoV< PseudoV<unsigned char> > > & visibilityMap, int lvl)
{
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return fullHide->ourImages[hideBitmap[x][y][lvl]].bitmap; //fully hidden
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[22].bitmap; //visible right bottom corner
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[15].bitmap; //visible right top corner
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[22].bitmap); //visible left bottom corner
		return partialHide->ourImages[34].bitmap; //visible left bottom corner
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[15].bitmap); //visible left top corner
		return partialHide->ourImages[35].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		//return partialHide->ourImages[rand()%2].bitmap; //visible top
		return partialHide->ourImages[0].bitmap; //visible top
	}
	else if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return partialHide->ourImages[4+rand()%2].bitmap; //visble bottom
		return partialHide->ourImages[4].bitmap; //visble bottom
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[2+rand()%2].bitmap); //visible left
		//return CSDL_Ext::rotate01(partialHide->ourImages[2].bitmap); //visible left
		return partialHide->ourImages[36].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		//return partialHide->ourImages[2+rand()%2].bitmap; //visible right
		return partialHide->ourImages[2].bitmap; //visible right
	}
	else if(visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl])
	{
		//return partialHide->ourImages[12+2*(rand()%2)].bitmap; //visible bottom, right - bottom, right; left top corner hidden
		return partialHide->ourImages[12].bitmap; //visible bottom, right - bottom, right; left top corner hidden
	}
	else if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[13].bitmap; //visible right, right - top; left bottom corner hidden
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && !visibilityMap[x+1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[13].bitmap); //visible top, top - left, left; right bottom corner hidden
		return partialHide->ourImages[37].bitmap;
	}
	else if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x+1][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[12+2*(rand()%2)].bitmap); //visible left, left - bottom, bottom; right top corner hidden
		//return CSDL_Ext::rotate01(partialHide->ourImages[12].bitmap); //visible left, left - bottom, bottom; right top corner hidden
		return partialHide->ourImages[38].bitmap;
	}
	else if(visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[10].bitmap; //visible left, right, bottom and top
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[16].bitmap; //visible right corners
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[18].bitmap; //visible top corners
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[16].bitmap); //visible left corners
		return partialHide->ourImages[39].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::hFlip(partialHide->ourImages[18].bitmap); //visible bottom corners
		return partialHide->ourImages[40].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[17].bitmap; //visible right - top and bottom - left corners
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::hFlip(partialHide->ourImages[17].bitmap); //visible top - left and bottom - right corners
		return partialHide->ourImages[41].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[19].bitmap; //visible corners without left top
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[20].bitmap; //visible corners without left bottom
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[20].bitmap); //visible corners without right bottom
		return partialHide->ourImages[42].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[19].bitmap); //visible corners without right top
		return partialHide->ourImages[43].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[21].bitmap; //visible all corners only
	}
	if(visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl])
	{
		return partialHide->ourImages[6].bitmap; //hidden top
	}
	if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl])
	{
		return partialHide->ourImages[7].bitmap; //hidden right
	}
	if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl])
	{
		return partialHide->ourImages[8].bitmap; //hidden bottom
	}
	if(visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[7].bitmap); //hidden left
		return partialHide->ourImages[44].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl])
	{
		return partialHide->ourImages[9].bitmap; //hidden top and bottom
	}
	if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl])
	{
		return partialHide->ourImages[29].bitmap;  //hidden left and right
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[24].bitmap; //visible top and right bottom corner
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[24].bitmap); //visible top and left bottom corner
		return partialHide->ourImages[45].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[33].bitmap; //visible top and bottom corners
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[26].bitmap); //visible left and right top corner
		return partialHide->ourImages[46].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[25].bitmap); //visible left and right bottom corner
		return partialHide->ourImages[47].bitmap;
	}
	if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl])
	{
		return partialHide->ourImages[32].bitmap; //visible left and right corners
	}
	if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[30].bitmap); //visible bottom and left top corner
		return partialHide->ourImages[48].bitmap;
	}
	if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y-1][lvl])
	{
		return partialHide->ourImages[30].bitmap; //visible bottom and right top corner
	}
	if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y-1][lvl])
	{
		return partialHide->ourImages[31].bitmap; //visible bottom and top corners
	}
	if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[25].bitmap; //visible right and left bottom corner
	}
	if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[26].bitmap; //visible right and left top corner
	}
	if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[32].bitmap); //visible right and left cornres
		return partialHide->ourImages[49].bitmap;
	}
	if(visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl])
	{
		return partialHide->ourImages[28].bitmap; //visible bottom, right - bottom, right; left top corner visible
	}
	else if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y+1][lvl])
	{
		return partialHide->ourImages[27].bitmap; //visible right, right - top; left bottom corner visible
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y+1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[27].bitmap); //visible top, top - left, left; right bottom corner visible
		return partialHide->ourImages[50].bitmap;
	}
	else if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x+1][y-1][lvl])
	{
		//return CSDL_Ext::rotate01(partialHide->ourImages[28].bitmap); //visible left, left - bottom, bottom; right top corner visible
		return partialHide->ourImages[51].bitmap;
	}
	//newly added
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible t and tr
	{
		return partialHide->ourImages[0].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible t and tl
	{
		return partialHide->ourImages[1].bitmap;
	}
	else if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible b and br
	{
		return partialHide->ourImages[4].bitmap;
	}
	else if(visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl]) //visible b and bl
	{
		return partialHide->ourImages[5].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible l and tl
	{
		return partialHide->ourImages[36].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && !visibilityMap[x+1][y][lvl] && visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && visibilityMap[x-1][y+1][lvl]) //visible l and bl
	{
		return partialHide->ourImages[36].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && !visibilityMap[x+1][y+1][lvl] && visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible r and tr
	{
		return partialHide->ourImages[2].bitmap;
	}
	else if(!visibilityMap[x][y+1][lvl] && visibilityMap[x+1][y][lvl] && !visibilityMap[x-1][y][lvl] && !visibilityMap[x][y-1][lvl] && !visibilityMap[x-1][y-1][lvl] && visibilityMap[x+1][y+1][lvl] && !visibilityMap[x+1][y-1][lvl] && !visibilityMap[x-1][y+1][lvl]) //visible r and br
	{
		return partialHide->ourImages[3].bitmap;
	}
	return fullHide->ourImages[0].bitmap; //this case should never happen, but it is better to hide too much than reveal it....
}

int CMapHandler::getCost(int3 &a, int3 &b, const CGHeroInstance *hero)
{
	int ret=-1;
	if(a.x>=CGI->mh->reader->map.width && a.y>=CGI->mh->reader->map.height)
		ret = hero->type->heroClass->terrCosts[CGI->mh->ttiles[CGI->mh->reader->map.width-1][CGI->mh->reader->map.width-1][a.z].malle];
	else if(a.x>=CGI->mh->reader->map.width && a.y<CGI->mh->reader->map.height)
		ret = hero->type->heroClass->terrCosts[CGI->mh->ttiles[CGI->mh->reader->map.width-1][a.y][a.z].malle];
	else if(a.x<CGI->mh->reader->map.width && a.y>=CGI->mh->reader->map.height)
		ret = hero->type->heroClass->terrCosts[CGI->mh->ttiles[a.x][CGI->mh->reader->map.width-1][a.z].malle];
	else
		ret = hero->type->heroClass->terrCosts[CGI->mh->ttiles[a.x][a.y][a.z].malle];
	if(!(a.x==b.x || a.y==b.y))
		ret*=1.41421;

	//TODO: use hero's pathfinding skill during calculating cost
	return ret;
}

std::vector < std::string > CMapHandler::getObjDescriptions(int3 pos)
{
	std::vector < std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> > objs = ttiles[pos.x][pos.y][pos.z].objects;
	std::vector<std::string> ret;
	for(int g=0; g<objs.size(); ++g)
	{
		if( (5-(objs[g].first->pos.y-pos.y)) >= 0 && (5-(objs[g].first->pos.y-pos.y)) < 6 && (objs[g].first->pos.x-pos.x) >= 0 && (objs[g].first->pos.x-pos.x)<7 && objs[g].first->defInfo &&
			(((objs[g].first->defInfo->blockMap[5-(objs[g].first->pos.y-pos.y)])>>((objs[g].first->pos.x-pos.x)))&1)==0
			) //checking position blocking
		{
			//unsigned char * blm = objs[g].first->defInfo->blockMap;
			if (objs[g].first->state)
				ret.push_back(objs[g].first->state->hoverText(objs[g].first));
			else
				ret.push_back(CGI->objh->objects[objs[g].first->ID].name);
		}
	}
	return ret;
}

std::vector < CGObjectInstance * > CMapHandler::getVisitableObjs(int3 pos)
{
	std::vector < CGObjectInstance * > ret;
	for(int h=0; h<ttiles[pos.x][pos.y][pos.z].objects.size(); ++h)
	{
		CGObjectInstance * curi = ttiles[pos.x][pos.y][pos.z].objects[h].first;
		if(curi->visitableAt(- curi->pos.x + pos.x + curi->getWidth() - 1, -curi->pos.y + pos.y + curi->getHeight() - 1))
			ret.push_back(curi);
	}
	return ret;
}

CGObjectInstance * CMapHandler::createObject(int id, int subid, int3 pos)
{
	CGObjectInstance * nobj;
	switch(id)
	{
	case 43: //hero
		nobj = new CGHeroInstance;
		break;
	case 98: //town
		nobj = new CGTownInstance;
		break;
	default: //rest of objects
		nobj = new CGObjectInstance;
		break;
	}
	nobj->ID = id;
	nobj->subID = subid;
	nobj->defInfo = CGI->dobjinfo->gobjs[id][subid];
	if(!nobj->defInfo)
		std::cout <<"No def declaration for " <<id <<" "<<subid<<std::endl;
		/*new CGDefInfo;
	int defObjInfoNumber = -1;
	for(int f=0; f<CGI->dobjinfo->objs.size(); ++f)
	{
		if(CGI->dobjinfo->objs[f].type==id && CGI->dobjinfo->objs[f].subtype == subid)
		{
			defObjInfoNumber = f;
			break;
		}
	}
	nobj->defInfo->name = CGI->dobjinfo->objs[defObjInfoNumber].defName;
	for(int g=0; g<6; ++g)
		nobj->defInfo->blockMap[g] = CGI->dobjinfo->objs[defObjInfoNumber].blockMap[g];
	for(int g=0; g<6; ++g)
		nobj->defInfo->visitMap[g] = CGI->dobjinfo->objs[nobj->defObjInfoNumber].visitMap[g];
	nobj->defInfo->printPriority = CGI->dobjinfo->objs[nobj->defObjInfoNumber].priority;*/
	nobj->pos = pos;
	//nobj->state = NULL;//new CLuaObjectScript();
	nobj->tempOwner = 254;
	nobj->info = NULL;
	nobj->defInfo->id = id;
	nobj->defInfo->subid = subid;

	//assigning defhandler

	std::string ourName = getDefName(id, subid);
	std::transform(ourName.begin(), ourName.end(), ourName.begin(), (int(*)(int))toupper);
	nobj->defInfo->name = ourName;

	if(loadedDefs[ourName] == NULL)
	{
		nobj->defInfo->handler = CGI->spriteh->giveDef(ourName);
		loadedDefs[ourName] = nobj->defInfo->handler;
	}
	else
	{
		nobj->defInfo->handler = loadedDefs[ourName];
	}

	return nobj;
}

std::string CMapHandler::getDefName(int id, int subid)
{
	CGDefInfo* temp = CGI->dobjinfo->gobjs[id][subid];
	if(temp)
		return temp->name;
	throw new std::exception("Def not found.");
}

bool CMapHandler::printObject(CGObjectInstance *obj)
{
	CDefHandler * curd = obj->defInfo->handler;
	for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
	{
		for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
		{
			SDL_Rect cr;
			cr.w = 32;
			cr.h = 32;
			cr.x = fx*32;
			cr.y = fy*32;
			std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> toAdd = std::make_pair(obj, std::make_pair(cr, std::vector<std::list<int3>>()));
			///initializing places that will be coloured by blitting (flag colour / player colour positions)
			if(CGI->dobjinfo->gobjs[toAdd.first->ID][toAdd.first->subID]->isVisitable())
			{
				toAdd.second.second.resize(toAdd.first->defInfo->handler->ourImages.size());
				for(int no = 0; no<toAdd.first->defInfo->handler->ourImages.size(); ++no)
				{
					bool breakNow = true;
					for(int dx=0; dx<32; ++dx)
					{
						for(int dy=0; dy<32; ++dy)
						{
							SDL_Surface * curs = toAdd.first->defInfo->handler->ourImages[no].bitmap;
							Uint32* point = (Uint32*)( (Uint8*)curs->pixels + curs->pitch * (fy*32+dy) + curs->format->BytesPerPixel*(fx*32+dx));
							Uint8 r, g, b, a;
							SDL_GetRGBA(*point, curs->format, &r, &g, &b, &a);
							if(r==255 && g==255 && b==0)
							{
								toAdd.second.second[no].push_back(int3((fx*32+dx), (fy*32+dy), 0));
								breakNow = false;
							}
						}
					}
					if(breakNow)
						break;
				}
			}
			if((obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)>=0 && (obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)<ttiles.size()-Woff && (obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)>=0 && (obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)<ttiles[0].size()-Hoff)
			{
				TerrainTile2 & curt = 
					ttiles
					  [obj->pos.x + fx - curd->ourImages[0].bitmap->w/32]
				      [obj->pos.y + fy - curd->ourImages[0].bitmap->h/32]
					  [obj->pos.z];


				ttiles[obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1][obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1][obj->pos.z].objects.push_back(toAdd);
			}

		} // for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
	} //for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
	return true;
}

bool CMapHandler::hideObject(CGObjectInstance *obj)
{
	CDefHandler * curd = obj->defInfo->handler;
	for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
	{
		for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
		{
			if((obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)>=0 && (obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1)<ttiles.size()-Woff && (obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)>=0 && (obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1)<ttiles[0].size()-Hoff)
			{
				std::vector < std::pair<CGObjectInstance*,std::pair<SDL_Rect, std::vector<std::list<int3>>>> > & ctile = ttiles[obj->pos.x + fx - curd->ourImages[0].bitmap->w/32+1][obj->pos.y + fy - curd->ourImages[0].bitmap->h/32+1][obj->pos.z].objects;
				for(int dd=0; dd<ctile.size(); ++dd)
				{
					if(ctile[dd].first->id==obj->id)
						ctile.erase(ctile.begin() + dd);
				}
			}

		} // for(int fy=0; fy<curd->ourImages[0].bitmap->h/32; ++fy)
	} //for(int fx=0; fx<curd->ourImages[0].bitmap->w/32; ++fx)
	return true;
}

std::string CMapHandler::getRandomizedDefName(CGDefInfo *di, CGObjectInstance * obj)
{
	return std::string();
}

bool CMapHandler::removeObject(CGObjectInstance *obj)
{
	hideObject(obj);
	std::vector<CGObjectInstance *>::iterator db = std::find(CGI->objh->objInstances.begin(), CGI->objh->objInstances.end(), obj);
	recalculateHideVisPosUnderObj(*db);
	delete *db;
	CGI->objh->objInstances.erase(db);
	return true;
}

bool CMapHandler::recalculateHideVisPos(int3 &pos)
{
	ttiles[pos.x][pos.y][pos.z].visitable = false;
	ttiles[pos.x][pos.y][pos.z].blocked = false;
	for(int i=0; i<ttiles[pos.x][pos.y][pos.z].objects.size(); ++i)
	{
		CDefHandler * curd = ttiles[pos.x][pos.y][pos.z].objects[i].first->defInfo->handler;
		for(int fx=0; fx<8; ++fx)
		{
			for(int fy=0; fy<6; ++fy)
			{
				int xVal = ttiles[pos.x][pos.y][pos.z].objects[i].first->pos.x + fx - 7;
				int yVal = ttiles[pos.x][pos.y][pos.z].objects[i].first->pos.y + fy - 5;
				int zVal = ttiles[pos.x][pos.y][pos.z].objects[i].first->pos.z;
				if(xVal>=0 && xVal<ttiles.size()-Woff && yVal>=0 && yVal<ttiles[0].size()-Hoff)
				{
					TerrainTile2 & curt = ttiles[xVal][yVal][zVal];
					if(((ttiles[pos.x][pos.y][pos.z].objects[i].first->defInfo->visitMap[fy] >> (7 - fx)) & 1))
						curt.visitable = true;
					if(!((ttiles[pos.x][pos.y][pos.z].objects[i].first->defInfo->blockMap[fy] >> (7 - fx)) & 1))
						curt.blocked = true;
				}
			}
		}
	}
	return true;
}

bool CMapHandler::recalculateHideVisPosUnderObj(CGObjectInstance *obj, bool withBorder)
{
	if(withBorder)
	{
		for(int fx=-1; fx<=obj->defInfo->handler->ourImages[0].bitmap->w/32; ++fx)
		{
			for(int fy=-1; fy<=obj->defInfo->handler->ourImages[0].bitmap->h/32; ++fy)
			{
				if((obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32+1)>=0 && (obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32+1)<ttiles.size()-Woff && (obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32+1)>=0 && (obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32+1)<ttiles[0].size()-Hoff)
				{
					recalculateHideVisPos(int3(obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32 +1, obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32 + 1, obj->pos.z));
				}
			}
		}
	}
	else
	{
		for(int fx=0; fx<obj->defInfo->handler->ourImages[0].bitmap->w/32; ++fx)
		{
			for(int fy=0; fy<obj->defInfo->handler->ourImages[0].bitmap->h/32; ++fy)
			{
				if((obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32+1)>=0 && (obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32+1)<ttiles.size()-Woff && (obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32+1)>=0 && (obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32+1)<ttiles[0].size()-Hoff)
				{
					recalculateHideVisPos(int3(obj->pos.x + fx - obj->defInfo->handler->ourImages[0].bitmap->w/32 +1, obj->pos.y + fy - obj->defInfo->handler->ourImages[0].bitmap->h/32 + 1, obj->pos.z));
				}
			}
		}
	}
	return true;
}

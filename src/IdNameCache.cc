/*
 * lftp and utils
 *
 * Copyright (c) 2001 by Alexander V. Lukyanov (lav@yars.free.net)
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <ctype.h>
#include "IdNameCache.h"
#include "xmalloc.h"

void IdNameCache::init()
{
   memset(table_id,0,sizeof(table_id));
   memset(table_name,0,sizeof(table_name));
}
void IdNameCache::free_list(IdNamePair *list)
{
   while(list)
   {
      IdNamePair *next=list->next;
      delete list;
      list=next;
   }
}
void IdNameCache::free()
{
   for(int i=0; i<table_size; i++)
      free_list(table_id[i]);
}
void IdNameCache::add(unsigned h,IdNamePair **p,IdNamePair *r)
{
   r->next=p[h];
   p[h]=r;
}
IdNamePair *IdNameCache::lookup(int id)
{
   unsigned h=hash(id);
   for(IdNamePair *scan=table_id[h]; scan; scan=scan->next)
      if(id==scan->id)
	 return scan;
   IdNamePair *r=get_record(id);
   if(!r)
      r=new IdNamePair(id,0);
   add(h,table_id,r);
   if(r->name)
      add(hash(r->name),table_name,r);
   return r;
}
IdNamePair *IdNameCache::lookup(const char *name)
{
   if(isdigit((unsigned char)*name))
      return lookup(atoi(name));
   unsigned h=hash(name);
   for(IdNamePair *scan=table_id[h]; scan; scan=scan->next)
      if(!strcmp(name,scan->name))
	 return scan;
   IdNamePair *r=get_record(name);
   if(!r)
      r=new IdNamePair(-1,name);
   add(h,table_name,r);
   if(r->id!=-1)
      add(hash(r->id),table_id,r);
   return r;
}
const char *IdNameCache::Lookup(int id)
{
   const char *name=lookup(id)->name;
   if(name && name[0])
      return name;
   static char buf[32];
   sprintf(buf,"%d",id);
   return buf;
}
int IdNameCache::Lookup(const char *name)
{
   if(isdigit((unsigned char)*name))
      return atoi(name);
   return lookup(name)->id;
}
IdNameCache::~IdNameCache()
{
   free();
   Delete(expire_timer);
}
int IdNameCache::Do()
{
   if(expire_timer && expire_timer->Stopped())
      deleting=true;
   return STALL;
}

unsigned IdNameCache::hash(int id)
{
   return unsigned(id)%table_size;
}
unsigned IdNameCache::hash(const char *name)
{
   unsigned h=0;
   while(*name)
      h+=(h<<4)+*name++;
   return h%table_size;
}

IdNamePair *PasswdCache::get_record(int id)
{
   struct passwd *p=getpwuid(id);
   if(!p)
      return 0;
   return new IdNamePair(p->pw_uid,p->pw_name);
}
IdNamePair *GroupCache::get_record(int id)
{
   struct group *p=getgrgid(id);
   if(!p)
      return 0;
   return new IdNamePair(p->gr_gid,p->gr_name);
}
IdNamePair *PasswdCache::get_record(const char *name)
{
   struct passwd *p=getpwnam(name);
   if(!p)
      return 0;
   return new IdNamePair(p->pw_uid,p->pw_name);
}
IdNamePair *GroupCache::get_record(const char *name)
{
   struct group *p=getgrnam(name);
   if(!p)
      return 0;
   return new IdNamePair(p->gr_gid,p->gr_name);
}

PasswdCache *PasswdCache::instance;
GroupCache *GroupCache::instance;
PasswdCache *PasswdCache::GetInstance()
{
   if(instance)
      return instance;
   instance=new PasswdCache();
   instance->SetExpireTimer(new Timer(3));
   return instance;
}
GroupCache *GroupCache::GetInstance()
{
   if(instance)
      return instance;
   instance=new GroupCache();
   instance->SetExpireTimer(new Timer(3));
   return instance;
}
PasswdCache::~PasswdCache()
{
   if(this==instance)
      instance=0;
}
GroupCache::~GroupCache()
{
   if(this==instance)
      instance=0;
}
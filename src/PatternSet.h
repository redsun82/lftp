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

/* $Id$ */

#ifndef EXCLUDE_H
#define EXCLUDE_H

#include <sys/types.h>
CDECL_BEGIN
#include <regex.h>
CDECL_END

class PatternSet
{
public:
   enum Type { EXCLUDE,INCLUDE };

private:
   class Pattern
   {
   protected:
      char *pattern;
   public:
      virtual bool Match(const char *str)=0;
      Pattern(const char *str);
      virtual ~Pattern();
   };

   class PatternLink
   {
      friend class PatternSet;
      Type type;
      Pattern *pattern;
      PatternLink *next;
      PatternLink(Type t,Pattern *p,PatternLink *n)
	 {
	    type=t;
	    pattern=p;
	    next=n;
	 }
      ~PatternLink()
	 {
	    delete pattern;
	 }
   };

   PatternLink *chain;
   PatternLink **add;

public:
   PatternSet();
   ~PatternSet();

   void	Add(Type,Pattern *);

   bool Match(Type t,const char *str) const;
   bool MatchExclude(const char *str) const { return Match(EXCLUDE,str); }
   bool MatchInclude(const char *str) const { return Match(INCLUDE,str); }

   class Regex : public Pattern
   {
      regex_t compiled;
      char *error;
   public:
      Regex(const char *str);
      bool Error() { return error!=0; }
      const char *ErrorText() { return error; }
      bool Match(const char *str);
      ~Regex();
   };
   class Glob : public Pattern
   {
      int slash_count;
   public:
      Glob(const char *p);
      bool Match(const char *str);
   };
};

#endif
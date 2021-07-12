/*
 * BerylDB - A lightweight database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2021 - Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include "beryl.h"
#include "core_keys.h"

class CoreModKeys : public Module
{
    private:
    
        CommandSet cmdset;
        CommandGet cmdget;
        CommandStrlen cmdstrlen;
        CommandKeys cmdkeys;
        CommandCount cmdcount;
        CommandGetDel cmdgetdel;
        CommandGetSet cmdgetset;
        CommandGetSubstr cmdsubstr;
        CommandRKey cmdrkey;
        CommandTouch cmdtouch;
        CommandNTouch cmdntouch;
        CommandAppend cmdappend;
        CommandSetNX cmdsetnx;
        CommandSetTX cmdsettx;
        CommandSearch cmdsearch;
        CommandWDel cmdwdel;
        CommandGetOccurs cmdgetoccurs;
        CommandGetExp cmdgetexp;
        CommandIsAlpha cmdisalpha;
        CommandIsNum cmdisnum;
        CommandGetPersist cmdgetpersist;
        
    public:	
        
        CoreModKeys() : cmdset(this), 
                        cmdget(this),	
                        cmdstrlen(this),
                        cmdkeys(this),
                        cmdcount(this),
                        cmdgetdel(this),
                        cmdgetset(this),
                        cmdsubstr(this),
                        cmdrkey(this),
                        cmdtouch(this),
                        cmdntouch(this),
                        cmdappend(this),
                        cmdsetnx(this),
                        cmdsettx(this),
                        cmdsearch(this),
                        cmdwdel(this),
                        cmdgetoccurs(this),
                        cmdgetexp(this),
                        cmdisalpha(this),
                        cmdisnum(this),
                        cmdgetpersist(this)
        {
        
        }
        
        Version GetDescription() 
        {
                return Version("Provides commands related to keys, such as SET, DEL and GET.", VF_BERYLDB|VF_CORE);
        }
};

MODULE_LOAD(CoreModKeys)

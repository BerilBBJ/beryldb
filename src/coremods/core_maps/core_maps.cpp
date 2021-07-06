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
#include "core_maps.h"

class CoreModuleMaps : public Module
{
    private:
    
        CommandHSet cmdhset;
        CommandHGet cmdhget;
        CommandHDel cmdhdel;
        CommandHCount cmdhcount;
        CommandHFind cmdhfind;
        CommandHList cmdhlist;
        CommandHExists cmdhexists;
        CommandHStrlen cmdhstrlen;
        CommandHSetNX cmdhsetnx;
        CommandHWDel cmdhwdel;
        
    public:	
        
        CoreModuleMaps() : cmdhset(this), 
                           cmdhget(this),
                           cmdhdel(this),
                           cmdhcount(this),
                           cmdhfind(this),
                           cmdhlist(this),
                           cmdhexists(this),
                           cmdhstrlen(this),
                           cmdhsetnx(this),
                           cmdhwdel(this)
                           
                           
        {
        
        }
        
        Version GetDescription() 
        {
                return Version("Provides commands to handle maps.", VF_BERYLDB|VF_CORE);
        }
};

MODULE_LOAD(CoreModuleMaps)

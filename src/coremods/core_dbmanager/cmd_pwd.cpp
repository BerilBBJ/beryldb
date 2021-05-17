/*
 * BerylDB - A modular database.
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
#include "brldb/dbmanager.h"
#include "brldb/dbnumeric.h"
#include "brldb/database.h"
#include "brldb/query.h"
#include "converter.h"
#include "engine.h"
#include "core_dbmanager.h"

CommandPWD::CommandPWD(Module* Creator) : Command(Creator, "PWD", 0)
{
       requires = 'e';
}

COMMAND_RESULT CommandPWD::Handle(User* user, const Params& parameters)
{  
       const std::string path = Kernel->Store->Default->path.c_str();
       
       user->SendProtocol(BRLD_CURRENT_DIR, path, Daemon::Format("Database path: %s", path.c_str()).c_str());
       return SUCCESS;
}
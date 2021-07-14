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
#include "core_info.h"

CommandModules::CommandModules(Module* parent) : ServerTargetCommand(parent, "MODULES")
{
	group = 'w';
	syntax = "[<server>]";
}

COMMAND_RESULT CommandModules::Handle(User* user, const Params& parameters)
{
	bool for_us = (parameters.empty() || engine::equals(parameters[0], Kernel->Config->ServerName));

	if ((!for_us) || (!IS_LOCAL(user)))
	{
		if (!user->IsAdmin())
		{
			user->SendProtocol(BRLD_ALERT, "Access denied.");
			return FAILED;
		}

		if (!for_us)
		{
			return SUCCESS;
		}
	}

        Dispatcher::JustAPI(user, BRLD_BEGIN_OF_MODLIST);

	const ModuleHandler::ModuleMap& mods = Kernel->Modules->GetModules();

  	for (ModuleHandler::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
	{
		Module* m = i->second;
		Version V = m->GetDescription();

                bool is_core = false;

		if (IS_LOCAL(user) && user->IsAdmin())
		{
			std::string flags("VCO");
			size_t pos = 0;
	
			for (int mult = 1; mult <= VF_OPTCOMMON; mult *= 2, ++pos)
			{
				if (!(V.Flags & mult))
				{
					flags[pos] = '-';
				}
                                else
                                {
                                        if (mult == VF_CORE && is_core == false)
                                        {
                                             is_core = true;
                                        }
                                }
                        }
                        
                        if (is_core)
                        {
                              continue;
                        }

			user->SendRemoteProtocol(BRLD_INPUT, BRLD_MODLIST, Daemon::Format("%-20s | %5s", m->SourceFile.c_str(), V.description.c_str()));
		}
		else
		{
			user->SendRemoteProtocol(BRLD_INPUT, BRLD_MODLIST, Daemon::Format("%-20s | %5s", m->SourceFile.c_str(), V.description.c_str()));
		}
	}
	
        Dispatcher::JustAPI(user, BRLD_END_OF_MODLIST);
	return SUCCESS;
}

CommandCoreModules::CommandCoreModules(Module* parent) : ServerTargetCommand(parent, "COREMODULES")
{
	group = 'w';
	syntax = "[<server>]";
}

COMMAND_RESULT CommandCoreModules::Handle(User* user, const Params& parameters)
{
	bool for_us = (parameters.empty() || engine::equals(parameters[0], Kernel->Config->ServerName));

	if ((!for_us) || (!IS_LOCAL(user)))
	{
		if (!user->IsAdmin())
		{
			user->SendProtocol(ERR_INPUT, ACCESS_DENIED);
			return FAILED;
		}

		if (!for_us)
		{
			return SUCCESS;
		}
	}

        Dispatcher::JustAPI(user, BRLD_BEGIN_OF_MODLIST);

	const ModuleHandler::ModuleMap& mods = Kernel->Modules->GetModules();

  	for (ModuleHandler::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
	{
		Module* m = i->second;
		Version V = m->GetDescription();
		
		bool is_core = false;
		
		if (IS_LOCAL(user) && user->IsAdmin())
		{
			std::string flags("VCO");
			size_t pos = 0;
	
			for (int mult = 1; mult <= VF_OPTCOMMON; mult *= 2, ++pos)
			{
				if (!(V.Flags & mult))
				{
					flags[pos] = '-';
				}
				else
				{
		                        if (mult == VF_CORE && is_core == false)
		                        {
	        	                     is_core = true;
	                	        }
	                        }
			}
			
			if (!is_core)
			{
			      continue;
			}
			
			user->SendRemoteProtocol(BRLD_INPUT, BRLD_MODLIST, Daemon::Format("%-20s | %5s", m->SourceFile.c_str(), V.description.c_str()));
		}
		else
		{
			user->SendRemoteProtocol(BRLD_INPUT, BRLD_MODLIST, Daemon::Format("%-20s | %5s", m->SourceFile.c_str(), V.description.c_str()));
		}
	}
	
        Dispatcher::JustAPI(user, BRLD_END_OF_MODLIST);
	return SUCCESS;
}

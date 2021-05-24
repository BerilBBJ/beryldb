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

#pragma once

#include "subscription.h"

/*
 * Class Channel maintains data of a given channel. This class
 * handles relevant data, such as subscribed users, name, and provides
 * basic functionalities, such as join and part functions.
 */

class Externalize Channel : public Expandable
{
  friend class ChannelManager;
  friend class User;
  
  public:
	
	/* Subscription map typedef. */
	
 	typedef std::map<User*, brld::aligned_storage<Subscription> > SubscriptionMap;

  private:
	
	/* 
	 * DeleteUser is invoked from PartUser() and removes references
	 * of a given subscription in this->subscribedlist.
	 */

	void DeleteUser(const SubscriptionMap::iterator& subsiter);
	
	/* Deletes an user from subscribedlist. */

	void DeleteUser(User* user);

  public:
	
	/* This channel's name. */

	std::string name;
	
	/* Time at which this channel was created. */

	time_t created;
	
	/* Map that contains all subscribed clients to a given chan. */
	
	SubscriptionMap subscribedlist;

	/* Channel constructor. */
	
	Channel(const std::string &name, time_t current);
	
	/* 
	 * Returns a counter of all clients subscribed (joined)
	 * in a given channel.
	 */
	 
	size_t GetClientCounter() const 
	{ 
		return subscribedlist.size(); 
	}

	/*
	 * Adds a user to the reference list.
	 * 
	 * @parameters:
	 *           · User*: user to add.
	 */

	Subscription* AddUser(User* user);

	/* Returns a keyed map of subscribed users */

	const SubscriptionMap& GetInstances() const 
	{ 
		return subscribedlist; 
	}

	/* 
	 * Checks whether an user is subscribed to a channel. 
         *
         * @parameters:
	 *
	 *         · User: User to verify if subscribed,
	 * 
         * @return:
 	 *
         *         · True: User is subscribed.
         *         · False: User is not subscribed.
         */    

	bool IsSubscribed(User* user);

	/* Returns subscription data for a given user. */

	Subscription* GetUser(User* user);

	/* 
	 * Parts an user from channel.
         *
         * @parameters:
	 *
	 *         · User: User to unsubscribe.
	 * 
         * @return:
 	 *
         *         · True: OK
         *         · False: Operation failed.
         */    	
	
	bool PartUser(User* user);
	
	/* Joins an user to a given channel.*/

	static Channel* JoinUser(bool fromconnect, LocalUser* user, std::string channame, bool override = false);

	/* Creates a channel if not exists. */
	
	Subscription* ExecuteJoin(bool fromconnect, User* user, bool bursting = false, bool created_by_local = false);
	
	void Write(ProtocolTrigger::Event& protoev, char status = 0, const DiscardList& except_list = DiscardList(), User* joining = NULL);

	void Write(ProtocolTrigger::EventProvider& protoevprov, ProtocolTrigger::Message& msg, char status = 0, const DiscardList& except_list = DiscardList(), User* joining = NULL);
};

inline bool Channel::IsSubscribed(User* user)
{
	return (this->subscribedlist.find(user) != this->subscribedlist.end());
}

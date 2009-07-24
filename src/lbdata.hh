/*
 * Module: lbdata.hh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */
#ifndef __LBDATA_HH__
#define __LBDATA_HH__

#include <assert.h>
#include <map>
#include <set>
#include <vector>
#include <string>

using namespace std;

class LBHealth;

/**
 *
 *
 **/
class LBRule {
 public:
  typedef map<string, int> InterfaceDistColl;
  typedef map<string, int>::iterator InterfaceDistIter;

  typedef enum {ALL,ICMP,UDP,TCP} Protocol;
  typedef enum {K_SECOND,K_MINUTE,K_HOUR} LimitPeriod;

  LBRule() :
    _proto("all"),
    _exclude(false),
    _failover(false),
    _enable_source_based_routing(false),
    _limit(false),
    _limit_burst("5"),
    _limit_rate("1"),
    _limit_mode(false),
    _limit_period(K_HOUR)
  {}

 public:
  string _proto;
  string _s_addr;
  string _s_port;
  string _s_port_ipt;

  string _d_addr;
  string _d_port;
  string _d_port_ipt;

  bool _exclude;
  bool _failover;

  bool _enable_source_based_routing;

  bool _limit;
  string _limit_burst;
  string _limit_rate;
  bool _limit_mode; //true above, false below
  LimitPeriod _limit_period;

  string _in_iface;
  InterfaceDistColl _iface_dist_coll;
};


/**
 *
 *
 **/
class LBHealthHistory {
public:
  LBHealthHistory(int buffer_size);

  //push in the ping response for this...
  int push(int rtt);

public:
  //results of health testing
  unsigned long _last_success;
  unsigned long _last_failure;

  unsigned long _failure_count;

  static int _buffer_size;
  vector<int> _resp_data;
  int _index;
};

/**
 *
 *
 **/
class LBTest {
public:
  typedef enum {K_NONE,K_SUCCESS,K_FAILURE} TestState;
public:
  LBTest(bool debug) : 
    _debug(debug),
    _state(K_NONE)
  {}
  virtual ~LBTest() {}

  virtual void
  init();
  
  virtual void
  send(LBHealth &health) = 0;

  virtual int
  recv(LBHealth &health) = 0;

  virtual string
  dump() = 0;

public:
  bool _debug;
  string _target;
  int    _resp_time;
  int _state;
  static int _recv_icmp_sock;
  static int _send_raw_sock;
  static int _send_icmp_sock;
  static bool _initialized;
};

/**
 *
 *
 **/
class LBHealth {
public:
  typedef map<int,LBTest*> TestColl;
  typedef map<int,LBTest*>::iterator TestIter;
  typedef map<int,LBTest*>::const_iterator TestConstIter;

public:
  LBHealth() :
    _success_ct(0),
    _failure_ct(0),
    _hresults(10),
    _is_active(true),
    _state_changed(true),
    _last_time_state_changed(0),
    _interface_index(0)
      {}

  LBHealth(int interface_index, string &interface) :
    _success_ct(0),
    _failure_ct(0),
    _hresults(10),
    _is_active(true),
    _state_changed(true),
    _last_time_state_changed(0),
    _interface(interface),
    _interface_index(interface_index)
      {}

  void put(int rtt);

  bool
  state_changed() const {return _state_changed;}

  unsigned long
  last_success() const {return _hresults._last_success;}

  unsigned long
  last_failure() const {return _hresults._last_failure;}

  unsigned long
  failure_count() const {return _failure_ct;}

  //test interfaces
  void
  start_new_test_cycle();

  void
  send_test();

  bool
  recv_test();

public: //variables
  int _success_ct;
  int _failure_ct;
  string _nexthop;
  string _dhcp_nexthop;
  LBHealthHistory _hresults;
  bool _is_active;
  bool _state_changed;
  unsigned long _last_time_state_changed;
  string _interface;
  int _interface_index;
  string _address;

  TestColl _test_coll;
private: //variables
  TestIter _test_iter;
  bool _test_success;
};

/**
 *
 *
 **/
class LBData {
 public:
  typedef map<int,LBRule> LBRuleColl;
  typedef map<int,LBRule>::iterator LBRuleIter;
  typedef map<int,LBRule>::const_iterator LBRuleConstIter;
  typedef map<string,LBHealth> InterfaceHealthColl;
  typedef map<string,LBHealth>::iterator InterfaceHealthIter;
  typedef map<string,LBHealth>::const_iterator InterfaceHealthConstIter;

  LBData() : _disable_source_nat(false),_flush_conntrack(false) {}

  bool
  error() {return false;}

  bool
  is_active(const string &iface);

  map<string,string>
  state_changed();

  void
  reset_state_changed();

  void
  update_dhcp_nexthop();

  void
  dump();

 public:
  string _filename;

  LBRuleColl _lb_rule_coll;
  InterfaceHealthColl _iface_health_coll;

  bool _disable_source_nat;
  bool _flush_conntrack;
  string _hook;
};

#endif //__LBDATA_HH__

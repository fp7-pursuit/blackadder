/* 
 * File:   client_exec.hpp
 * Author: Dimitris Syrivelis
 * Created on aug 2, 2011, 11:10 AM
 */

#ifndef CLIENT_EXEC_HPP
#define	CLIENT_EXEC_HPP


#include "parser.hpp"
#include "network.hpp"

using namespace std;

class ClientInstance;

class ClientInstance {
public:
    string testbed_ip;
    string labelconf;
    string cmd_line;
    int epoch;
};

class ClientExec {

public:  
   ClientExec(Domain * _dm) {  dm = _dm; }  
   ~ClientExec() { ; }
   int number_of_instances;
   vector<ClientInstance *> instances;

   int addInstance(const Setting &instance);
   int LoadCfg(string filename);
   void DeployExperiment();
   Config cfg;
   Domain * dm;
   string deploy_server_ip;
};


#endif	/* CLIENT_EXEC */


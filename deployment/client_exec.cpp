/* 
 * File:   client_exec.cpp
 * Author: Dimitris Syrivelis
 * Created on August 2, 2011, 11:10 AM
 */

#include "client_exec.hpp"
#include <sstream>

int ClientExec::addInstance(const Setting &instance)
{
    int ret;
    ClientInstance *ci = new ClientInstance();
    string testbed_ip;
    string labelconf;
    string cmd_line;
    string epoch;
 
    if (instance.lookupValue("testbed_ip", testbed_ip)) {
        cout << "This instance will be deployed on " << testbed_ip << endl;
	ci->testbed_ip = testbed_ip;
    } else {
        cerr << "error no testbed_ip - don't know where to deploy this instance" << endl;
	return -1;
    }
    if (instance.lookupValue("labelconf", labelconf)) {
        ci->labelconf = labelconf;
    } else {
        cerr << "labelconf is missing from node and is mandatory for stdout file naming" << endl;
        return -1;
    }
    if (instance.lookupValue("cmd_line", cmd_line)) {
	ci->cmd_line = cmd_line;
    } else {
	cerr << "cmd_line is mandatory, what am i deploying?" << endl; 
    }
    if (instance.lookupValue("epoch", epoch)) {
	ci->epoch = atoi(epoch.c_str());
    } else {
	cerr << "epoch is mandatory " << endl; 
    }
 
    instances.push_back(ci);
    return 0;


}

int ClientExec::LoadCfg(string filename)
{
    int ret;

    try {
        cfg.readFile(filename.c_str());
        cout << "I read client exec configuration " << filename << endl;
    } catch (const FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        return -1;
    } catch (const ParseException &pex) {
        return -1;
    }

    try {
        cfg.lookupValue("DEPLOY_SERVER_IP",deploy_server_ip);
    } catch (const SettingNotFoundException &nfex) {
        cerr << "mandatory option DEPLOY_SERVER_IP is missing" << endl;
        return -1;
    }

    const Setting& root = cfg.getRoot();
    try {
        const Setting &instance_map = root["experiment"]["instances"];
        number_of_instances = instance_map.getLength();
        cout << "Number of instances in the experiment file: " << number_of_instances << endl;
        for (int i = 0; i < number_of_instances; i++) {
            const Setting &instance = instance_map[i];
            ret = addInstance(instance);
            if (ret < 0) {
                return -1;
            }
        }
    } catch (const SettingNotFoundException &nfex) {
        return -1;
        cerr << "SettingNotFoundException" << endl;
    }

   cout << EOF;    
}

void ClientExec::DeployExperiment()
{
  //Start server netcats
  //netcat -l -p 1111 > ./out.file
  string netcat_srv;
  string command;
  system ("killall -9 netcat");
  for (int i = 10000; i< (10000+number_of_instances); i++) {
	std::stringstream out;
	out << i;
	netcat_srv = string("netcat -l -p ") +out.str()+string(" > ") + dm->write_conf + instances[i-10000]->labelconf + string(".out &");
	system(netcat_srv.c_str());
  }

  for (int i = 10000; i< (10000+number_of_instances); i++) {
	std::stringstream out;
	out << i;

        if (dm->sudo) {
                command = "ssh " + dm->user + "@" + instances[i-10000]->testbed_ip + " \"sudo " + string("netcat ")+deploy_server_ip +" "+out.str()+ " -e \'"+instances[i-10000]->cmd_line+"\'\" &";
		system(command.c_str());
        } else {
                command = "ssh " + dm->user + "@" + instances[i-10000]->testbed_ip + " \"" + string("netcat ")+deploy_server_ip +" "+out.str()+ " -e \'"+instances[i-10000]->cmd_line+"\'\" &";
                system(command.c_str());
        }
  }
   
  //TODO add support for netcat
  //netcat 192.168.130.61 1111 -e 'dist/Debug/GNU-Linux-x86/deployment -c planetlab.cfg -a'

}





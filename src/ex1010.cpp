/*
 * simple MOOSApp ping pong example
 */

#include "MOOS/libMOOS/App/MOOSApp.h"

class ExampleApp : public CMOOSApp
{
	bool OnProcessCommandLine(){
		pinger_ = m_CommandLineParser.GetFlag("--ping");

		burstsize_ =1000;
		m_CommandLineParser.GetVariable("--burst_size",burstsize_);

		SetMOOSName(GetAppName()+ std::string((pinger_ ? "ping":"pong")));

		return true;
	}

	void OnPrintHelpAndExit(){
		std::cerr<<"\nSimple flat-out ping-pong app:\n"
				"in different terminals launch\n"
				"   ./ex1010 --ping --burst_size=500\n"
				"and then\n"
				"   ./ex1010 --pong\n";

	}

	bool OnStartUp(){
		count_=0;mean_latency_=0.0;max_latency_=0.0;
		SetAppFreq(2,0.0);
		return SetIterateMode(REGULAR_ITERATE_AND_COMMS_DRIVEN_MAIL);
	}

	bool OnNewMail(MOOSMSG_LIST & Mail){
		//process it
		MOOSMSG_LIST::iterator q;
		for(q=Mail.begin();q!=Mail.end();q++)
		{
			//are we a ponger
			if(q->GetKey()=="ex1010-ping")
				Notify("ex1010-pong",q->GetDouble());

			if(q->GetKey()=="ex1010-pong"){
				double latency =(MOOSLocalTime()-q->GetDouble())/2;
				mean_latency_+= latency;
				if(count_>2)//this simply removes case of stale data in DB
					max_latency_ = std::max(latency,max_latency_);
				if(count_<burstsize_-1)
					Notify("ex1010-ping",MOOSLocalTime());
			}
			count_++;
		}
		return true;
	}

	bool OnConnectToServer(){
		if(pinger_)
			return Register("ex1010-pong",0.0);
		else
			return Register("ex1010-ping",0.0);
	}

	bool Iterate()
	{
		if(pinger_)
		{
			std::cout<<"ping-ponged "<<count_<<" messages ("<<2*count_<<") messages exchanged\n";
			std::cout<<" mean latency of "<<mean_latency_*1000000.0/count_<<" us\n";
			std::cout<<" max  latency of "<<max_latency_*1000000.0<<" us\n";

			//excite the system once in while...
			Notify("ex1010-ping",MOOSLocalTime());

			count_ = 0;mean_latency_ = 0.0;
		}
		return true;
	}

	bool pinger_;
	unsigned int count_;
	unsigned int burstsize_;
	double mean_latency_;
	double max_latency_;
};

int main(int argc, char * argv[])
{
	//here we do some command line parsing...
	MOOS::CommandLineParser P(argc,argv);

	//mission file could be first free parameter
	std::string mission_file = P.GetFreeParameter(0, "Mission.moos");

	//app name can be the  second free parameter
	std::string app_name = P.GetFreeParameter(1, "ex1010");

	ExampleApp App;

	return App.Run(app_name,mission_file,argc,argv);
}

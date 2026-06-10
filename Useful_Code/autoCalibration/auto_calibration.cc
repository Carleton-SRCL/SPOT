/* This code records RAW data of the 4 markers present on the corners of 
the spacecraft. It saves the data into a text file that is read by matlab in
order to determine where the centre of mass of the platform is with 
respect to the 4 LEDs*/

#include <iostream>
#include <cstring>
#include <fstream> //for ofstream
#include "owl.hpp"


using namespace std;

int main(int argc, const char **argv)
{
	int idx = 0; // row position
	int idx2 = 0; // column position of each marker
	ofstream groundTruthArm("auto_calibrate_servicer2.txt"); // title of the output text file
	const std::string myoptions = "profile=all120 frequency=20"; // options for initializing
	const int frequency = 20; // for calculating time
	const int duration = 30; // number of seconds to record data
	double data[frequency*duration][13]; // holder for the data

	const int ButterFilter = 0; // keep filter off for auto calibration!

	// filter storage variables
	double ang_prev_in_1 = 0;
	double ang_prev_in_2 = 0;
	double ang_prev_out_1 = 0;
	double ang_prev_out_2 = 0;
	double x_prev_in_1 = 0;
	double x_prev_in_2 = 0;
	double x_prev_out_1 = 0;
	double x_prev_out_2 = 0;
	double y_prev_in_1 = 0;
	double y_prev_in_2 = 0;
	double y_prev_out_1 = 0;
	double y_prev_out_2 = 0;
	double temp;

	string address = argc > 1 ? argv[1] : "192.168.1.109";
	OWL::Context owl;
	OWL::Markers markers;

	if (owl.open(address) <= 0 || owl.initialize(myoptions) <= 0)
	{
		return 0; //quit the program if a connection cannot be established
	}

	// start streaming
	owl.streaming(1);

	// main loop
	bool timeInit = false;
	while (owl.isOpen() && owl.property<int>("initialized") && idx <= frequency*duration)
	{
		const OWL::Event *event = owl.nextEvent(1000);
		if (!event) continue;

		if (idx % frequency == 0 && timeInit)
		{
			cout << "Seconds Remaining" << double(idx - frequency*duration) / double(frequency) << endl;
		}

		if (event->type_id() == OWL::Type::ERROR)
		{
			cerr << event->name() << ": " << event->str() << endl;
			break;
		}
		else if (event->type_id() == OWL::Type::FRAME)
		{
			if (event->find("markers", markers) > 0)
			{
				idx2 = 0; // column position of each marker
				data[idx][0] = double(event->time()) / double(frequency);
				for (OWL::Markers::iterator m = markers.begin(); m != markers.end(); m++)
				{
					if (m->cond > 0)
					{
						//cout << m->cond << endl;

						if (!timeInit)
						{
							idx = 0; // row position
							timeInit = true;
							cout << "Spacecraft should already be spinning! Repeat if not the case" << endl;
							//usleep(10000000);
						}

						data[idx][0] = double(event->time()) / double(frequency);

						if (m->id == 7 || m->id == 1 || m->id == 3 || m->id == 5)
						{

							data[idx][idx2 * 3 + 1] = m->x; // x
							data[idx][idx2 * 3 + 2] = m->y; // y
							data[idx][idx2 * 3 + 3] = m->z; // z

							if (ButterFilter == 1)
							{
								// filtering angle 
								temp = data[idx][4];
								data[idx][4] = 0.7478*ang_prev_out_1 - 0.2722*ang_prev_out_2 + 0.1311*data[idx][4] + 0.2622*ang_prev_in_1 + 0.1311*ang_prev_in_2;
								ang_prev_out_2 = ang_prev_out_1;
								ang_prev_out_1 = data[idx][4];
								ang_prev_in_2 = ang_prev_in_1;
								ang_prev_in_1 = temp;

								// filtering x
								temp = data[idx][1];
								data[idx][1] = 0.7478*x_prev_out_1 - 0.2722*x_prev_out_2 + 0.1311*data[idx][1] + 0.2622*x_prev_in_1 + 0.1311*x_prev_in_2;
								x_prev_out_2 = x_prev_out_1;
								x_prev_out_1 = data[idx][1];
								x_prev_in_2 = x_prev_in_1;
								x_prev_in_1 = temp;

								// filtering y
								temp = data[idx][2];
								data[idx][2] = 0.7478*y_prev_out_1 - 0.2722*y_prev_out_2 + 0.1311*data[idx][2] + 0.2622*y_prev_in_1 + 0.1311*y_prev_in_2;
								y_prev_out_2 = y_prev_out_1;
								y_prev_out_1 = data[idx][2];
								y_prev_in_2 = y_prev_in_1;
								y_prev_in_1 = temp;

							}
							idx2++;
						}
						
					}
					
				}
				idx++;
			}
		}//if

	} // while
	owl.done();
	owl.close();

	cout << "Writing data to file...";
	for (int i = 0; i != frequency*duration; i++)
	{
		groundTruthArm << data[i][0] << "	" << data[i][1] << "	" << data[i][2] << "	" << data[i][3] << "	" << data[i][4] << "	" << data[i][5] << "	" << data[i][6] << "	" << data[i][7] << "	" << data[i][8] << "	" << data[i][9] << "	" << data[i][10] << "	" << data[i][11] << "	" << data[i][12] << endl;
		
	}
	cout << "Done!";
	groundTruthArm.close(); // closing text file
	return 0;
}

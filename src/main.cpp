/*
 * main.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: nbingham
 */

#include <parse/parse.h>
#include <parse_hse/parallel.h>
#include <hse/graph.h>
#include <hse/simulator.h>
#include <interpret_hse/import.h>
#include <interpret_dot/export.h>
#include <interpret_boolean/export.h>
#include <boolean/variable.h>

void print_help()
{
	cout << "Usage: hsesim [options] file..." << endl;
	cout << "Options:" << endl;
	cout << " -h,--help      Display this information" << endl;
	cout << "    --version   Display version information" << endl;
	cout << " -v,--verbose   Display verbose messages" << endl;
	cout << " -d,--debug     Display internal debugging messages" << endl;
	cout << endl;
	cout << " -o             Specify the output file name" << endl;
	cout << "    formats other than 'dot' are passed onto graphviz dot for rendering" << endl;
	cout << " -s,--sync      Render half synchronization actions" << endl;
}

void print_version()
{
	cout << "hsesim 1.0.0" << endl;
	cout << "Copyright (C) 2013 Sol Union." << endl;
	cout << "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
	cout << endl;
}

int main(int argc, char **argv)
{
	configuration config;
	config.set_working_directory(argv[0]);
	tokenizer tokens;
	parse_hse::parallel::register_syntax(tokens);

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "--help" || arg == "-h")			// Help
			print_help();
		else if (arg == "--version")	// Version Information
			print_version();
		else if (arg == "--verbose" || arg == "-v")
			set_verbose(true);
		else if (arg == "--debug" || arg == "-d")
			set_debug(true);
		else
			config.load(tokens, argv[i], "");
	}

	if (is_clean() && tokens.segments.size() > 0)
	{
		parse_hse::parallel syntax(tokens);

		boolean::variable_set v;
		hse::graph g = import_graph(tokens, syntax, v, true);
		g.compact();
		cout << syntax.to_string() << endl;
		cout << "reset: " << export_disjunction(g.reset, v).to_string() << endl;

		hse::simulator sim(&g);

		int enabled = 1;
		int step = 0;
		string command = "";
		bool done = false;
		while (!done)
		{
			cout << "(hsesim)";
			cout.flush();

			cin >> command;

			if (command == "help")
			{
				cout << "help\t\tPrint this message" << endl;
				cout << "step <N>\t\tStep through N transitions" << endl;
				cout << "reset\t\tReset the simulator to the initial marking" << endl;
			}
			else if (command == "step")
			{
				int steps = 0;
				cin >> steps;
				for (int i = 0; i < steps && enabled != 0; i++)
				{
					enabled = sim.enabled();

					if (enabled == 0)
						error("", "deadlock detected", __FILE__, __LINE__);
					else
					{
						int firing = rand()%enabled;
						if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::active)
							printf("%d\tT%d.%d\t%s\n", step, sim.local.ready[firing].index, sim.local.ready[firing].term, export_disjunction(g.transitions[sim.local.ready[firing].index].action[sim.local.ready[firing].term], v).to_string().c_str());
						else if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::passive)
							printf("%d\tT%d.%d\t[%s]\n", step, sim.local.ready[firing].index, sim.local.ready[firing].term, export_disjunction(g.transitions[sim.local.ready[firing].index].action[sim.local.ready[firing].term], v).to_string().c_str());
						sim.fire(firing);
					}
					step++;
				}
			}
			else if (command == "quit")
				done = true;


		}
	}

	complete();
	return is_clean();
}

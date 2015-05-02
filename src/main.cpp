/*
 * main.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: nbingham
 */

#include <common/standard.h>
#include <parse/parse.h>
#include <parse_hse/parallel.h>
#include <hse/graph.h>
#include <hse/simulator.h>
#include <interpret_hse/import.h>
#include <interpret_dot/export.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <boolean/variable.h>

void print_help()
{
	printf("Usage: hsesim [options] file...\n");
	printf("A simulation environment for HSE processes.\n");
	printf("Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -v,--verbose   Display verbose messages\n");
	printf(" -d,--debug     Display internal debugging messages\n");
	printf("\n");
	printf("The following flags implement any graph conversion that requires simulation of some kind. If none of these flags are set, then an interactive simulation environment is executed.\n");
	printf(" -g <file>      Convert this HSE to an hse-graph and save it to a file\n");
	printf(" -eg <file>     Convert this HSE to an elaborated hse-graph and save it to a file\n");
	printf(" -pn <file>     Convert this HSE to a petri-net and save it to a file\n");
	printf(" -sg <file>     Convert this HSE to a state-graph and save it to a file\n");
}

void print_version()
{
	printf("hsesim 1.0.0\n");
	printf("Copyright (C) 2013 Sol Union.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

void print_command_help()
{
	printf("<arg> specifies a required argument\n(arg=value) specifies an optional argument with a default value\n");
	printf("\n== General ==\n");
	printf("  help, h             print this message\n");
	printf("  seed <n>            set the random seed for the simulation\n");
	printf("  source <file>       source and execute a list of commands from a file\n");
	printf("  save <file>         save the sequence of fired transitions to a file\n");
	printf("  load <file>         load a sequence of transitions from a file\n");
	printf("  unload              return to random stepping");
	printf("  quit, q             exit the interactive simulation environment\n");
	printf("\n== Running Simulation ==\n");
	printf("  tokens, t           list the location and state information of every token\n");
	printf("  enabled, e          return the list of enabled transitions\n");
	printf("  fire <i>, f<i>      fire the i'th enabled transition\n");
	printf("  step (N=1), s(N=1)  step through N transitions (random unless a sequence is loaded)\n");
	printf("  reset, r            reset the simulator to the initial marking and re-seed\n");
	printf("\n== Setting/Viewing State ==\n");
	printf("  set <i> <expr>      execute a transition as if it were local to the i'th token\n");
	printf("  set <expr>          execute a transition as if it were remote to all tokens\n");
	printf("  force <expr>        execute a transition as if it were local to all tokens\n");
}

void real_time(hse::graph &g, boolean::variable_set &v)
{
	hse::simulator sim(&g);

	tokenizer conjunction_parser(false);
	parse_boolean::internal_parallel::register_syntax(conjunction_parser);

	int seed = 0;
	srand(seed);
	int enabled = sim.enabled();
	vector<pair<int, int> > steps;
	int step = 0;
	int n = 0, n1 = 0;
	char command[256];
	bool done = false;
	FILE *script = stdin;
	while (!done)
	{
		printf("(hsesim)");
		fflush(stdout);
		if (fgets(command, 255, script) == NULL && script != stdin)
		{
			fclose(script);
			script = stdin;
			fgets(command, 255, script);
		}
		int length = strlen(command);
		command[length-1] = '\0';
		length--;

		if ((strncmp(command, "help", 4) == 0 && length == 4) || (strncmp(command, "h", 1) == 0 && length == 1))
			print_command_help();
		else if ((strncmp(command, "quit", 4) == 0 && length == 4) || (strncmp(command, "q", 1) == 0 && length == 1))
			done = true;
		else if (strncmp(command, "seed", 4) == 0)
		{
			if (sscanf(command, "seed %d", &n) == 1)
				seed = n;
			else
				printf("error: expected seed value\n");
		}
		else if (strncmp(command, "unload", 6) == 0 && length == 6)
			steps.resize(step);
		else if (strncmp(command, "source", 6) == 0 && length > 7)
		{
			script = fopen(&command[7], "r");
			if (script == NULL)
			{
				printf("error: file not found '%s'", &command[7]);
				script = stdin;
			}
		}
		else if (strncmp(command, "load", 4) == 0 && length > 5)
		{
			FILE *seq = fopen(&command[5], "r");
			if (seq != NULL)
			{
				while (fgets(command, 255, seq) != NULL)
				{
					if (sscanf(command, "%d.%d", &n, &n1) == 2)
						steps.push_back(pair<int, int>(n, n1));
				}
				fclose(seq);
			}
		}
		else if (strncmp(command, "save", 4) == 0 && length > 5)
		{
			FILE *seq = fopen(&command[5], "w");
			if (seq != NULL)
			{
				for (int i = 0; i < (int)steps.size(); i++)
					fprintf(seq, "%d.%d\n", steps[i].first, steps[i].second);
				fclose(seq);
			}
		}
		else if ((strncmp(command, "reset", 5) == 0 && length == 5) || (strncmp(command, "r", 1) == 0 && length == 1))
		{
			sim = hse::simulator(&g);
			enabled = sim.enabled();
			step = 0;
			srand(seed);
		}
		else if ((strncmp(command, "tokens", 6) == 0 && length == 6) || (strncmp(command, "t", 1) == 0 && length == 1))
		{
			for (int i = 0; i < sim.local.tokens.size(); i++)
				printf("(%d) P%d:%s     ", i, sim.local.tokens[i].index, export_conjunction(sim.local.tokens[i].state, v).to_string().c_str());
			printf("\n");
		}
		else if ((strncmp(command, "enabled", 7) == 0 && length == 7) || (strncmp(command, "e", 1) == 0 && length == 1))
		{
			for (int i = 0; i < enabled; i++)
			{
				if (g.transitions[sim.local.ready[i].index].behavior == hse::transition::active)
					printf("(%d) T%d.%d:%s     ", i, sim.local.ready[i].index, sim.local.ready[i].term, export_internal_parallel(g.transitions[sim.local.ready[i].index].action[sim.local.ready[i].term], v).to_string().c_str());
				else
					printf("(%d) T%d.%d:[%s]     ", i, sim.local.ready[i].index, sim.local.ready[i].term, export_conjunction(g.transitions[sim.local.ready[i].index].action[sim.local.ready[i].term], v).to_string().c_str());
			}
			cout << endl;
		}
		else if (strncmp(command, "set", 3) == 0)
		{
			if (sscanf(command, "set %d ", &n) != 1)
				n = -1;

			int i = 3;
			for (; i < length && n != -1 && command[i] != ' '; i++);

			conjunction_parser.insert("", string(command).substr(i+1));
			parse_boolean::internal_parallel expr(conjunction_parser);
			boolean::cube action = import_cube(conjunction_parser, expr, v, false);
			if (conjunction_parser.is_clean())
				for (int i = 0; i < (int)sim.local.tokens.size(); i++)
				{
					if (i == n)
						sim.local.tokens[i].state = boolean::local_transition(sim.local.tokens[i].state, action);
					else
						sim.local.tokens[i].state = boolean::remote_transition(sim.local.tokens[i].state, action);
				}
			conjunction_parser.reset();
			enabled = sim.enabled();
		}
		else if (strncmp(command, "force", 5) == 0)
		{
			if (length <= 6)
				printf("error: expected expression\n");
			else
			{
				conjunction_parser.insert("", string(command).substr(6));
				parse_boolean::internal_parallel expr(conjunction_parser);
				boolean::cube action = import_cube(conjunction_parser, expr, v, false);
				if (conjunction_parser.is_clean())
					for (int i = 0; i < (int)sim.local.tokens.size(); i++)
						sim.local.tokens[i].state = boolean::local_transition(sim.local.tokens[i].state, action);
				conjunction_parser.reset();
				enabled = sim.enabled();
			}
		}
		else if (strncmp(command, "step", 4) == 0 || strncmp(command, "s", 1) == 0)
		{
			if (sscanf(command, "step %d", &n) != 1 && sscanf(command, "s%d", &n) != 1)
				n = 1;

			for (int i = 0; i < n && enabled != 0; i++)
			{
				int firing = rand()%enabled;
				if (step < (int)steps.size())
				{
					for (firing = 0; firing < (int)sim.local.ready.size() &&
					(sim.local.ready[firing].index != steps[step].first || sim.local.ready[firing].term != steps[step].second); firing++);

					if (firing == (int)sim.local.ready.size())
					{
						printf("error: loaded simulation does not match HSE, please unload the simulation to continue\n");
						break;
					}
				}
				else
					steps.push_back(pair<int, int>(sim.local.ready[firing].index, sim.local.ready[firing].term));

				if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::active)
					printf("%d\tT%d.%d:%s\n", step, sim.local.ready[firing].index, sim.local.ready[firing].term, export_disjunction(g.transitions[sim.local.ready[firing].index].action[sim.local.ready[firing].term], v).to_string().c_str());
				else if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::passive)
					printf("%d\tT%d.%d:[%s]\n", step, sim.local.ready[firing].index, sim.local.ready[firing].term, export_disjunction(g.transitions[sim.local.ready[firing].index].action[sim.local.ready[firing].term], v).to_string().c_str());
				sim.fire(firing);
				enabled = sim.enabled();
				step++;
			}
		}
		else if (strncmp(command, "fire", 4) == 0 || strncmp(command, "f", 1) == 0)
		{
			if (sscanf(command, "fire %d", &n) == 1 || sscanf(command, "f%d", &n) == 1)
			{
				if (n < enabled)
				{
					if (step < (int)steps.size())
						printf("error: deviating from loaded simulation, please unload the simulation to continue\n");
					else
					{
						steps.push_back(pair<int, int>(sim.local.ready[n].index, sim.local.ready[n].term));

						if (g.transitions[sim.local.ready[n].index].behavior == hse::transition::active)
							printf("%d\tT%d.%d:%s\n", step, sim.local.ready[n].index, sim.local.ready[n].term, export_disjunction(g.transitions[sim.local.ready[n].index].action[sim.local.ready[n].term], v).to_string().c_str());
						else if (g.transitions[sim.local.ready[n].index].behavior == hse::transition::passive)
							printf("%d\tT%d.%d:[%s]\n", step, sim.local.ready[n].index, sim.local.ready[n].term, export_disjunction(g.transitions[sim.local.ready[n].index].action[sim.local.ready[n].term], v).to_string().c_str());

						sim.fire(n);
						enabled = sim.enabled();
						step++;
					}
				}
				else
					printf("error: must be in the range [0,%d)\n", enabled);
			}
			else
				printf("error: expected ID in the range [0,%d)\n", enabled);
		}
		else if (length > 0)
			printf("error: unrecognized command '%s'\n", command);
	}
}

int main(int argc, char **argv)
{
	configuration config;
	config.set_working_directory(argv[0]);
	tokenizer tokens;
	parse_hse::parallel::register_syntax(tokens);
	string sgfilename = "";
	string pnfilename = "";
	string egfilename = "";
	string gfilename = "";

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
		else if (arg == "-g")
		{
			i++;
			if (i < argc)
				gfilename = argv[i];
			else
			{
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		}
		else if (arg == "-eg")
		{
			i++;
			if (i < argc)
				egfilename = argv[i];
			else
			{
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		}
		else if (arg == "-pn")
		{
			i++;
			if (i < argc)
				pnfilename = argv[i];
			else
			{
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		}
		else if (arg == "-sg")
		{
			i++;
			if (i < argc)
				sgfilename = argv[i];
			else
			{
				error("", "expected output filename", __FILE__, __LINE__);
				return 1;
			}
		}
		else
			config.load(tokens, argv[i], "");
	}

	if (is_clean() && tokens.segments.size() > 0)
	{
		parse_hse::parallel syntax(tokens);

		boolean::variable_set v;
		hse::graph g = import_graph(tokens, syntax, v, true);
		g.compact();

		if (gfilename != "")
		{
			g.elaborate();

			ofstream s(gfilename.c_str());
			s << export_graph(g, v, false).to_string();
			s.close();
		}

		if (egfilename != "")
		{
			g.elaborate();

			ofstream s(egfilename.c_str());
			s << export_graph(g, v).to_string();
			s.close();
		}

		if (pnfilename != "")
		{
			hse::graph pn = g.to_petri_net();

			ofstream s(pnfilename.c_str());
			s << export_graph(pn, v).to_string();
			s.close();
		}

		if (sgfilename != "")
		{
			hse::graph sg = g.to_state_graph();

			ofstream s(sgfilename.c_str());
			s << export_graph(sg, v).to_string();
			s.close();
		}

		if (sgfilename == "" && pnfilename == "" && egfilename == "" && gfilename == "")
			real_time(g, v);
	}

	complete();
	return is_clean();
}

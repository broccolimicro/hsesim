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
#include <interpret_hse/export.h>
#include <interpret_dot/export.h>
#include <interpret_dot/import.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <boolean/variable.h>

void print_help()
{
	printf("Usage: hsesim [options] file...\n");
	printf("A simulation environment for HSE processes.\n");
	printf("\nSupported file formats:\n");
	printf(" *.hse           Load an HSE\n");
	printf(" *.dot           Load any place-transition graph\n");
	printf(" *.sim           Load a sequence of transitions to operate on\n");
	printf("\nGeneral Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -v,--verbose   Display verbose messages\n");
	printf(" -d,--debug     Display internal debugging messages\n");
	printf("\nConversion Options:\n");
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
	printf("\nGeneral:\n");
	printf(" help, h             print this message\n");
	printf(" seed <n>            set the random seed for the simulation\n");
	printf(" source <file>       source and execute a list of commands from a file\n");
	printf(" save <file>         save the sequence of fired transitions to a '.sim' file\n");
	printf(" load <file>         load a sequence of transitions from a '.sim' file\n");
	printf(" clear, c            clear any stored sequence and return to random stepping\n");
	printf(" quit, q             exit the interactive simulation environment\n");
	printf("\nRunning Simulation:\n");
	printf(" tokens, t           list the location and state information of every token\n");
	printf(" enabled, e          return the list of enabled transitions\n");
	printf(" fire <i>, f<i>      fire the i'th enabled transition\n");
	printf(" step (N=1), s(N=1)  step through N transitions (random unless a sequence is loaded)\n");
	printf(" reset (i), r(i)     reset the simulator to the initial marking and re-seed (does not clear)\n");
	printf("\nSetting/Viewing State:\n");
	printf(" set <i> <expr>      execute a transition as if it were local to the i'th token\n");
	printf(" set <expr>          execute a transition as if it were remote to all tokens\n");
	printf(" force <expr>        execute a transition as if it were local to all tokens\n");
}

void real_time(hse::graph &g, boolean::variable_set &v, vector<hse::term_index> steps = vector<hse::term_index>())
{
	hse::simulator sim;
	sim.base = &g;

	tokenizer internal_parallel_parser(false);
	parse_boolean::internal_parallel::register_syntax(internal_parallel_parser);

	int seed = 0;
	srand(seed);
	int enabled = 0;
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
		else if ((strncmp(command, "clear", 5) == 0 && length == 5) || (strncmp(command, "c", 1) == 0 && length == 1))
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
						steps.push_back(hse::term_index(n, n1));
				}
				fclose(seq);
			}
			else
				printf("error: file not found '%s'\n", &command[5]);
		}
		else if (strncmp(command, "save", 4) == 0 && length > 5)
		{
			FILE *seq = fopen(&command[5], "w");
			if (seq != NULL)
			{
				for (int i = 0; i < (int)steps.size(); i++)
					fprintf(seq, "%d.%d\n", steps[i].index, steps[i].term);
				fclose(seq);
			}
		}
		else if (strncmp(command, "reset", 5) == 0 || strncmp(command, "r", 1) == 0)
		{
			if (sscanf(command, "reset %d", &n) == 1 || sscanf(command, "r%d", &n) == 1)
			{
				sim = hse::simulator(&g, n);
				enabled = sim.enabled();
				step = 0;
				srand(seed);
			}
			else
			{
				for (int i = 0; i < (int)g.source.size(); i++)
				{
					printf("(%d) {", i);
					for (int j = 0; j < (int)g.source[i].size(); j++)
					{
						if (j != 0)
							printf(" ");
						printf("\tP%d:%s\n", g.source[i][j].index, export_conjunction(g.source[i][j].state, v).to_string().c_str());
					}
					printf("}\n");
				}
			}
		}
		else if ((strncmp(command, "tokens", 6) == 0 && length == 6) || (strncmp(command, "t", 1) == 0 && length == 1))
		{
			printf("%s {\n", export_conjunction(sim.global, v).to_string().c_str());
			for (int i = 0; i < sim.tokens.size(); i++)
				printf("\t(%d) P%d:%s\n", i, sim.tokens[i].index, export_conjunction(sim.tokens[i].state, v).to_string().c_str());
			printf("}\n");
		}
		else if ((strncmp(command, "enabled", 7) == 0 && length == 7) || (strncmp(command, "e", 1) == 0 && length == 1))
		{
			for (int i = 0; i < enabled; i++)
			{
				if (g.transitions[sim.ready[i].index].behavior == hse::transition::active)
					printf("(%d) T%d.%d:%s     ", i, sim.ready[i].index, sim.ready[i].term, export_internal_parallel(g.transitions[sim.ready[i].index].action[sim.ready[i].term], v).to_string().c_str());
				else
					printf("(%d) T%d.%d:[%s]     ", i, sim.ready[i].index, sim.ready[i].term, export_conjunction(g.transitions[sim.ready[i].index].action[sim.ready[i].term], v).to_string().c_str());
			}
			printf("\n");
		}
		else if (strncmp(command, "set", 3) == 0)
		{
			int i = 0;
			if (sscanf(command, "set %d ", &n) != 1)
			{
				n = -1;
				i = 4;
			}
			else
			{
				i = 5;
				while (i < length && command[i-1] != ' ')
					i++;
			}

			internal_parallel_parser.insert("", string(command).substr(i));
			parse_boolean::internal_parallel expr(internal_parallel_parser);
			boolean::cube action = import_cube(internal_parallel_parser, expr, v, false);
			if (internal_parallel_parser.is_clean())
				for (int i = 0; i < (int)sim.tokens.size(); i++)
				{
					if (i == n)
						sim.tokens[i].state = boolean::local_transition(sim.tokens[i].state, action);
					else
						sim.tokens[i].state = boolean::remote_transition(sim.tokens[i].state, action);
				}
			internal_parallel_parser.reset();
			enabled = sim.enabled();
		}
		else if (strncmp(command, "force", 5) == 0)
		{
			if (length <= 6)
				printf("error: expected expression\n");
			else
			{
				internal_parallel_parser.insert("", string(command).substr(6));
				parse_boolean::internal_parallel expr(internal_parallel_parser);
				boolean::cube action = import_cube(internal_parallel_parser, expr, v, false);
				if (internal_parallel_parser.is_clean())
					for (int i = 0; i < (int)sim.tokens.size(); i++)
						sim.tokens[i].state = boolean::local_transition(sim.tokens[i].state, action);
				internal_parallel_parser.reset();
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
					for (firing = 0; firing < (int)sim.ready.size() &&
					(sim.ready[firing].index != steps[step].index || sim.ready[firing].term != steps[step].term); firing++);

					if (firing == (int)sim.ready.size())
					{
						printf("error: loaded simulation does not match HSE, please clear the simulation to continue\n");
						break;
					}
				}
				else
					steps.push_back(hse::term_index(sim.ready[firing].index, sim.ready[firing].term));

				if (g.transitions[sim.ready[firing].index].behavior == hse::transition::active)
					printf("%d\tT%d.%d:%s\n", step, sim.ready[firing].index, sim.ready[firing].term, export_disjunction(g.transitions[sim.ready[firing].index].action[sim.ready[firing].term], v).to_string().c_str());
				else if (g.transitions[sim.ready[firing].index].behavior == hse::transition::passive)
					printf("%d\tT%d.%d:[%s]\n", step, sim.ready[firing].index, sim.ready[firing].term, export_disjunction(g.transitions[sim.ready[firing].index].action[sim.ready[firing].term], v).to_string().c_str());
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
						printf("error: deviating from loaded simulation, please clear the simulation to continue\n");
					else
					{
						steps.push_back(hse::term_index(sim.ready[n].index, sim.ready[n].term));

						if (g.transitions[sim.ready[n].index].behavior == hse::transition::active)
							printf("%d\tT%d.%d:%s\n", step, sim.ready[n].index, sim.ready[n].term, export_disjunction(g.transitions[sim.ready[n].index].action[sim.ready[n].term], v).to_string().c_str());
						else if (g.transitions[sim.ready[n].index].behavior == hse::transition::passive)
							printf("%d\tT%d.%d:[%s]\n", step, sim.ready[n].index, sim.ready[n].term, export_disjunction(g.transitions[sim.ready[n].index].action[sim.ready[n].term], v).to_string().c_str());

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
	tokenizer hse_tokens;
	tokenizer dot_tokens;
	parse_hse::parallel::register_syntax(hse_tokens);
	parse_dot::graph::register_syntax(dot_tokens);
	string sgfilename = "";
	string pnfilename = "";
	string egfilename = "";
	string gfilename = "";
	vector<hse::term_index> steps;

	bool labels = false;

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "--help" || arg == "-h")			// Help
		{
			print_help();
			return 0;
		}
		else if (arg == "--version")	// Version Information
		{
			print_version();
			return 0;
		}
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
		else if (arg == "--labels" || arg == "-l")
			labels = true;
		else
		{
			string filename = argv[i];
			int dot = filename.find_last_of(".");
			string format = "";
			if (dot != string::npos)
				format = filename.substr(dot+1);
			if (format == "hse")
				config.load(hse_tokens, filename, "");
			else if (format == "dot")
				config.load(dot_tokens, filename, "");
			else if (format == "sim")
			{
				FILE *seq = fopen(argv[i], "r");
				char command[256];
				int n, n1;
				if (seq != NULL)
				{
					while (fgets(command, 255, seq) != NULL)
					{
						if (sscanf(command, "%d.%d", &n, &n1) == 2)
							steps.push_back(hse::term_index(n, n1));
					}
					fclose(seq);
				}
				else
					printf("error: file not found '%s'\n", argv[i]);
			}
			else
				printf("unrecognized file format '%s'\n", format.c_str());
		}
	}

	if (is_clean() && hse_tokens.segments.size() > 0)
	{
		hse::graph g;
		boolean::variable_set v;

		hse_tokens.increment(false);
		hse_tokens.expect<parse_hse::parallel>();
		while (hse_tokens.decrement(__FILE__, __LINE__))
		{
			parse_hse::parallel syntax(hse_tokens);
			g.merge(hse::parallel, import_graph(hse_tokens, syntax, v, true));

			hse_tokens.increment(false);
			hse_tokens.expect<parse_hse::parallel>();
		}

		dot_tokens.increment(false);
		dot_tokens.expect<parse_dot::graph>();
		while (dot_tokens.decrement(__FILE__, __LINE__))
		{
			parse_dot::graph syntax(dot_tokens);
			g.merge(hse::parallel, import_graph(dot_tokens, syntax, v, true));

			dot_tokens.increment(false);
			dot_tokens.expect<parse_dot::graph>();
		}
		g.compact();

		if (gfilename != "")
		{
			FILE *fout = fopen(gfilename.c_str(), "w");
			fprintf(fout, "%s", export_graph(g, v, labels).to_string().c_str());
			printf("%s\n", export_parallel(g, v).to_string().c_str());
			fclose(fout);
		}

		if (egfilename != "")
		{
			g.elaborate();
			for (int i = 0; i < (int)g.places.size(); i++)
				g.places[i].predicate.espresso();

			FILE *fout = fopen(egfilename.c_str(), "w");
			fprintf(fout, "%s", export_graph(g, v, labels).to_string().c_str());
			fclose(fout);
		}

		if (pnfilename != "")
		{
			hse::graph pn = g.to_petri_net();

			FILE *fout = fopen(pnfilename.c_str(), "w");
			fprintf(fout, "%s", export_graph(pn, v, labels).to_string().c_str());
			fclose(fout);
		}

		if (sgfilename != "")
		{
			hse::graph sg = g.to_state_graph();

			FILE *fout = fopen(sgfilename.c_str(), "w");
			fprintf(fout, "%s", export_graph(sg, v, labels).to_string().c_str());
			fclose(fout);
		}

		if (sgfilename == "" && pnfilename == "" && egfilename == "" && gfilename == "")
			real_time(g, v, steps);
	}

	complete();
	return is_clean();
}

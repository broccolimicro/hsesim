/*
 * main.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: nbingham
 */

#include <common/standard.h>
#include <parse/parse.h>
#include <parse/default/block_comment.h>
#include <parse/default/line_comment.h>
#include <parse_chp/composition.h>
#include <hse/graph.h>
#include <hse/simulator.h>
#include <interpret_hse/import.h>
#include <interpret_hse/export.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <ucs/variable.h>

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

void real_time(hse::graph &g, ucs::variable_set &v, vector<hse::term_index> steps = vector<hse::term_index>())
{
	hse::simulator sim;
	sim.base = &g;
	sim.variables = &v;

	tokenizer assignment_parser(false);
	parse_expression::composition::register_syntax(assignment_parser);

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
			if (fgets(command, 255, script) == NULL)
				exit(0);
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
			{
				seed = n;
				srand(seed);
			}
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
				sim = hse::simulator(&g, &v, g.reset[n], 0, false);
				enabled = sim.enabled();
				step = 0;
				srand(seed);
			}
			else
				for (int i = 0; i < (int)g.reset.size(); i++)
					printf("(%d) %s\n", i, g.reset[i].to_string(v).c_str());
		}
		else if ((strncmp(command, "tokens", 6) == 0 && length == 6) || (strncmp(command, "t", 1) == 0 && length == 1))
		{
			vector<vector<int> > tokens;
			for (int i = 0; i < (int)sim.local.tokens.size(); i++)
			{
				bool found = false;
				for (int j = 0; j < (int)tokens.size() && !found; j++)
					if (g.places[sim.local.tokens[i].index].mask == g.places[sim.local.tokens[tokens[j][0]].index].mask)
					{
						tokens[j].push_back(i);
						found = true;
					}

				if (!found)
					tokens.push_back(vector<int>(1, i));
			}

			for (int i = 0; i < (int)tokens.size(); i++)
			{
				printf("%s {\n", export_expression(sim.encoding.flipped_mask(g.places[sim.local.tokens[tokens[i][0]].index].mask), v).to_string().c_str());
				for (int j = 0; j < (int)tokens[i].size(); j++)
					printf("\t(%d) P%d\t%s\n", tokens[i][j], sim.local.tokens[tokens[i][j]].index, export_node(hse::iterator(hse::place::type, sim.local.tokens[tokens[i][j]].index), g, v).c_str());
				printf("}\n");
			}
		}
		else if ((strncmp(command, "enabled", 7) == 0 && length == 7) || (strncmp(command, "e", 1) == 0 && length == 1))
		{
			for (int i = 0; i < enabled; i++)
			{
				if (g.transitions[sim.local.ready[i].index].behavior == hse::transition::active)
					printf("(%d) T%d.%d:%s     ", i, sim.local.ready[i].index, sim.local.ready[i].term, export_composition(g.transitions[sim.local.ready[i].index].local_action[sim.local.ready[i].term], v).to_string().c_str());
				else
					printf("(%d) T%d.%d:[%s]     ", i, sim.local.ready[i].index, sim.local.ready[i].term, export_expression(g.transitions[sim.local.ready[i].index].local_action[sim.local.ready[i].term], v).to_string().c_str());
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

			assignment_parser.insert("", string(command).substr(i));
			parse_expression::composition expr(assignment_parser);
			boolean::cube action = import_cube(expr, v, 0, &assignment_parser, false);
			boolean::cube remote_action = action.remote(v.get_groups());
			if (assignment_parser.is_clean())
			{
				sim.encoding = boolean::local_assign(sim.encoding, action, true);
				sim.global = boolean::local_assign(sim.global, remote_action, true);
				sim.encoding = boolean::remote_assign(sim.encoding, sim.global, true);
			}
			assignment_parser.reset();
			enabled = sim.enabled();
			sim.interference_errors.clear();
			sim.instability_errors.clear();
			sim.mutex_errors.clear();
		}
		else if (strncmp(command, "force", 5) == 0)
		{
			if (length <= 6)
				printf("error: expected expression\n");
			else
			{
				assignment_parser.insert("", string(command).substr(6));
				parse_expression::composition expr(assignment_parser);
				boolean::cube action = import_cube(expr, v, 0, &assignment_parser, false);
				boolean::cube remote_action = action.remote(v.get_groups());
				if (assignment_parser.is_clean())
				{
					sim.encoding = boolean::local_assign(sim.encoding, remote_action, true);
					sim.global = boolean::local_assign(sim.global, remote_action, true);
				}
				assignment_parser.reset();
				enabled = sim.enabled();
				sim.interference_errors.clear();
				sim.instability_errors.clear();
				sim.mutex_errors.clear();
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
					(sim.local.ready[firing].index != steps[step].index || sim.local.ready[firing].term != steps[step].term); firing++);

					if (firing == (int)sim.local.ready.size())
					{
						printf("error: loaded simulation does not match HSE, please clear the simulation to continue\n");
						break;
					}
				}
				else
					steps.push_back(hse::term_index(sim.local.ready[firing].index, sim.local.ready[firing].term));

				if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::active)
					printf("%d\tT%d.%d\t%s", step, sim.local.ready[firing].index, sim.local.ready[firing].term, export_composition(g.transitions[sim.local.ready[firing].index].local_action[sim.local.ready[firing].term], v).to_string().c_str());
				else if (g.transitions[sim.local.ready[firing].index].behavior == hse::transition::passive)
					printf("%d\tT%d\t[%s]", step, sim.local.ready[firing].index, export_expression(sim.local.ready[firing].guard_action, v).to_string().c_str());

				if (sim.local.ready[firing].vacuous)
					printf("\t\t\t[vacuous]");

				printf("\n");

				sim.fire(firing);

				enabled = sim.enabled();
				sim.interference_errors.clear();
				sim.instability_errors.clear();
				sim.mutex_errors.clear();
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
						steps.push_back(hse::term_index(sim.local.ready[n].index, sim.local.ready[n].term));

						if (g.transitions[sim.local.ready[n].index].behavior == hse::transition::active)
							printf("%d\tT%d.%d\t%s\n", step, sim.local.ready[n].index, sim.local.ready[n].term, export_composition(g.transitions[sim.local.ready[n].index].local_action[sim.local.ready[n].term], v).to_string().c_str());
						else if (g.transitions[sim.local.ready[n].index].behavior == hse::transition::passive)
							printf("%d\tT%d\t[%s]\n", step, sim.local.ready[n].index, export_expression(sim.local.ready[n].guard_action, v).to_string().c_str());

						sim.fire(n);

						enabled = sim.enabled();
						sim.interference_errors.clear();
						sim.instability_errors.clear();
						sim.mutex_errors.clear();
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
	parse_chp::composition::register_syntax(hse_tokens);
	parse_dot::graph::register_syntax(dot_tokens);
	hse_tokens.register_comment<parse::block_comment>();
	hse_tokens.register_comment<parse::line_comment>();
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
		ucs::variable_set v;

		bool first = true;
		hse_tokens.increment(false);
		hse_tokens.expect<parse_chp::composition>();
		while (hse_tokens.decrement(__FILE__, __LINE__))
		{
			parse_chp::composition syntax(hse_tokens);
			cout << syntax.to_string() << endl;
			g.merge(hse::parallel, import_graph(syntax, v, 0, &hse_tokens, true), !first);

			hse_tokens.increment(false);
			hse_tokens.expect<parse_chp::composition>();
			first = false;
		}

		dot_tokens.increment(false);
		dot_tokens.expect<parse_dot::graph>();
		while (dot_tokens.decrement(__FILE__, __LINE__))
		{
			parse_dot::graph syntax(dot_tokens);
			g.merge(hse::parallel, import_graph(syntax, v, &dot_tokens, true), !first);

			dot_tokens.increment(false);
			dot_tokens.expect<parse_dot::graph>();
			first = false;
		}
		g.post_process(v, true);
		g.check_variables(v);

		if (gfilename != "")
		{
			FILE *fout = fopen(gfilename.c_str(), "w");
			fprintf(fout, "%s", export_graph(g, v, labels).to_string().c_str());
			fclose(fout);
		}

		if (egfilename != "")
		{
			g.elaborate(v, true);

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
			hse::graph sg = g.to_state_graph(v);

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

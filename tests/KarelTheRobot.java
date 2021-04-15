// NOTE:
// The original file can be found here: https://www.complang.tuwien.ac.at/andi/KarelTheRobotNr.java.html
// To use it as a test case we will have to make some changes because it doesn't terminate.

/* Karel the Robot, a computer language learning game after Pattis            */
/* Author:      Andreas Krall                                                 */
/* Last Change: 96/03/28                                                      */

interface Globals {

	// error and return codes

	static final int simple_instr_finished     = -1;  // must be < 0
	static final int no_error                  =  0;  // must be 0
	static final int robot_made_turnoff        =  1;
	static final int double_definition_error   =  2;
	static final int incomplete_program_error  =  3;
	static final int blocked_robot_error       =  4;
	static final int end_of_world_error        =  5;
	static final int no_beeper_error           =  6;
	static final int empty_bag_error           =  7;
	static final int missing_turnoff_error     =  8;
	static final int back_end_reached_error    =  9;
	static final int stack_overflow_error      = 10;
	static final int internal_program_error    = 11;

	// test codes and names

	static final int undef_test           =  0;       // must be 0
	static final int front_is_clear       =  1;       // must be previous + 1
	static final int left_is_clear        =  2;
	static final int right_is_clear       =  3;
	static final int next_to_a_beeper     =  4;
	static final int facing_north         =  5;       // north must be east - 1
	static final int facing_east          =  6;       // east must be south - 1
	static final int facing_south         =  7;       // south must be west - 1
	static final int facing_west          =  8;       // west must be south + 1
	static final int any_beepers_in_bag   =  9;
	static final int front_is_blocked     = 10;
	static final int left_is_blocked      = 11;
	static final int right_is_blocked     = 12;
	static final int not_next_to_a_beeper = 13;
	static final int not_facing_north     = 14;
	static final int not_facing_east      = 15;
	static final int not_facing_south     = 16;
	static final int not_facing_west      = 17;
	static final int no_beepers_in_bag    = 18;
	
	static final String test_names[] = {
		"<test>",
		"front_is_clear",
		"left_is_clear",
		"right_is_clear",
		"next_to_a_beeper",
		"facing_north",
		"facing_east",
		"facing_south",
		"facing_west",
		"any_beepers_in_bag",
		"front_is_blocked",
		"left_is_blocked",
		"right_is_blocked",
		"not_next_to_a_beeper",
		"not_facing_north",
		"not_facing_east",
		"not_facing_south",
		"not_facing_west",
		"no_beepers_in_bag"
	};

	// basic instruction codes and names

	static final int undef_instr      = 0;     // must be 0
	static final int move_instr       = 1;     // must be previous + 1
	static final int turnleft_instr   = 2;
	static final int putbeeper_instr  = 3;
	static final int pickbeeper_instr = 4;
	static final int turnoff_instr    = 5;

	static final String instruction_names[] = {
		"<instr>", "move", "turnleft", "putbeeper", "pickbeeper", "turnoff"
	};

	// node codes and node element description codes

	static final int is_undef          =  0;   // must be less than is_number
	static final int program_node      =  1;   // must be less than is_number
	static final int define_node       =  2;   // must be less than is_number
	static final int execution_node    =  3;   // must be less than is_number
	static final int block_node        =  4;   // must be less than is_number
	static final int while_node        =  5;   // must be less than is_number
	static final int iterate_node      =  6;   // must be less than is_number
	static final int if_then_else_node =  7;   // must be less than is_number
	static final int call_node         =  8;   // must be less than is_number
	static final int basic_instr_node  =  9;   // must be less than is_number
	static final int is_program        = 10;   // must be less than is_number
	static final int is_execution      = 11;   // must be less than is_number
	static final int is_def_list       = 12;   // must be less than is_number
	static final int is_stmt           = 13;   // must be less than is_number
	static final int is_stmt_list      = 14;   // must be less than is_number
	static final int is_number         = 15; 
	static final int is_test           = 16;   // must be greater than is_number
	static final int is_name           = 17;   // must be greater than is_number
	static final int is_instr          = 18;   // must be greater than is_number
}


final class KarelTheRobot implements Globals{

	static int offset = 0;                     // offset of current instruction
	static Node instruction = null;            // current instruction

	// left searches to the next left position 

	static void left() {
		Node instr = instruction;

		if (offset == 0) {                        // find previous node in list
			while (instruction.offset == 0)       // find last node in list
				instruction = instruction.next;
			offset = instruction.offset - 1;      // set parent offset
			instruction = instruction.next;       // set parent node
			if (instruction.get_node_at_pos(offset) == instr)
				offset--;                         // original node is list head
			else {
				instruction = instruction.get_node_at_pos(offset);
				while (instruction.next != instr) // find previous node in list
					instruction = instruction.next;
				offset = instruction.length();    // offset of last element
			}
		} else
			offset--;
		if (offset == 0)
			return;
		// go down the tree to the rightmost node of the tree
		while ((instruction.description(offset) < is_number) &&
		       (instruction.get_node_at_pos(offset) != null)) {
		    // go down the tree
			instruction = instruction.get_node_at_pos(offset);
			while (instruction.offset == 0)       // find last node in list
				instruction = instruction.next;
			offset = instruction.length();        // offset of last element
		}
	}

	//right searches to next right position

	static void right() {
		offset++;
		while (offset >= instruction.length()) { // while behind last element
			offset = instruction.offset;         // set parents offset and
			instruction = instruction.next;      // go up to parent node
		}
		if ((offset != 0) &&                     // go down to child node
		    (instruction.description(offset) < is_number) &&
		    (instruction.get_node_at_pos(offset) != null)) {
			instruction = instruction.get_node_at_pos(offset);
			offset = 0;                          // first position in child node
		}
	}

	// insert_node insert a node at the current position

	static void insert_node(int node_code) {
		Node node, help;
		
		node = null;
		switch (node_code) {
			case program_node:
				offset = 0;
				instruction = new ProgramNode();
				return;
			case define_node:
				node = new DefineNode();
				break;
			case execution_node:
				node = new ExecutionNode();
				break;
			case block_node:
				node = new BlockNode();
				break;
			case while_node:
				node = new WhileNode();
				break;
			case iterate_node:
				node = new IterateNode();
				break;
			case if_then_else_node:
				node = new IfThenElseNode();
				break;
			case call_node:
				node = new CallNode();
				break;
			case basic_instr_node:
				node = new BasicInstrNode();
				break;
		}
		if (instruction.description(offset) == is_undef) {
			node.offset = instruction.offset;                 // middle of list
			node.next = instruction.next;
			instruction.offset = 0;
			instruction.next = node;
		} else if ((help = instruction.get_node_at_pos(offset)) == null) {
			instruction.put_object_at_pos(offset, node);      // empty
			node.offset = offset + 1;
			node.next = instruction;
		} else {
			instruction.put_object_at_pos(offset, node);      // head of list
			node.offset = help.offset;
			node.next = help.next;
		}
		offset = 0;
		instruction = node;
	}

	static public void print(int level, String text) {
		while (--level >= 0)
			System.out.print("   ");
		System.out.print(text);
	}

	static public void println(int level, String text) {
		while (--level >= 0)
			System.out.print("   ");
		System.out.println(text);
	}

	static public void print_instr(int level, Node instr) {
	
		if (instr == null)
			KarelTheRobot.println(level + 1, "<instruction>");
		else
			while (instr != null) {
				instr.print(level + 1);
				if (instr.offset != 0)
					return;
				instr = instr.next;
			}
	}

	static public void main(String args[]) 
	throws java.io.IOException {
		ProgramNode program;
		Node n1, n2, n3;
		IfThenElseNode if_stmt;
		int error;
		int ch;
		
		n1 = new BasicInstrNode(turnleft_instr);
		n1.next = new BasicInstrNode(turnleft_instr);
		n3 = n1.next.next = new BasicInstrNode(turnleft_instr);
		n2 = new BlockNode();
		((BlockNode) n2).instruction = n1;
		n3.next = n2;
		n3.offset = 2;
		n1 = new DefineNode();
		((DefineNode) n1).name =
		              Name.enter_name("turnright", (DefineNode) n1);
		((DefineNode) n1).instruction = n2;
		program = new ProgramNode();
		program.define = (DefineNode) n1;
		n2.offset = 3;
		n2.next = program.execution;
		program.execution.instruction = if_stmt = new IfThenElseNode();
		if_stmt.test = 3;
		if_stmt.next = new BasicInstrNode(turnoff_instr);
		if_stmt.next.offset = 2;
		if_stmt.next.next = program.execution;
		n1 = if_stmt.then_stmt = new CallNode();
		n1.offset = 3;
		n1.next = if_stmt;
		((CallNode) n1).definition = program.define.name;
		n1 = if_stmt.else_stmt = new IterateNode();
		n1.offset = 4;
		n1.next = if_stmt;
		((IterateNode) n1).count = 3;		

		program.print(0);

		offset = 0;
		instruction = program;

		BackBuffer.reset();
		DefineNode.reset();
		IterateNode.reset();
		
		ch = 'c';
		while ((error = instruction.exec_step()) <= 0) {
			//ch = System.in.read();
			if (ch != 'c')
				break;
		}
		
		System.out.println("Ergebnis des Programmes: " + error);
	}
}


final class KarelsWorld implements Globals{

    // world's bit 0 = north wall, bit 1 = east wall, bit 2 .. 15 = beeper count

	private static short world[][] = new short[100][100];
    private static final int north_wall = 1;
    private static final int east_wall = 2;

	private static int x = 0;                 // Karels x position
	private static int y = 0;                 // Karels y position
	private static int facing = facing_north; // Karels facing
	private static int bag = 0;               // number of beepers in Karels bag

	static void set_karel(int x, int y, int facing) {
		if (x < 0)
			x = 0;
		if (x >= world.length)
			x = world.length - 1;
		KarelsWorld.x = x;
		if (y < 0)
			y = 0;
		if (y >= world[0].length)
			y = world[0].length - 1;
		KarelsWorld.y = y;
		KarelsWorld.facing = facing_north;
		if ((facing == facing_north) ||
		    (facing == facing_east)  ||
		    (facing == facing_south) ||
		    (facing == facing_west))
			KarelsWorld.facing = facing;
	}

	static void fill_bag(int count) {
		if (count < 0)
			count = 0;
		bag = count;
	}

	private static boolean position_out_of_bounds(int x, int y) {
		if ((x < 0) || (x >= world.length) || (y < 0) || (y >= world[0].length))
			return true;
		return false;
	}

	static void set_north_wall(int x, int y) {
		if (position_out_of_bounds(x, y))
			return;
		world[x][y] |= north_wall;
	}

	static void set_east_wall(int x, int y) {
		if (position_out_of_bounds(x, y))
			return;
		world[x][y] |= east_wall;
	}

	static void clear_north_wall(int x, int y) {
		if (position_out_of_bounds(x, y))
			return;
		world[x][y] &= ~north_wall;
	}

	static void clear_east_wall(int x, int y) {
		if (position_out_of_bounds(x, y))
			return;
		world[x][y] &= ~east_wall;
	}

	static boolean test(int test) {
		switch (test) {
			case front_is_clear:
				switch (facing){
					case facing_north:
						return ((world[x][y] & north_wall) == 0);
					case facing_east:
						return ((world[x][y] & east_wall) == 0);
					case facing_south:
						if (y == 0)
							return false;
						return ((world[x][y-1] & north_wall) == 0);
					case facing_west:
						if (x == 0)
							return false;
						return ((world[x-1][y] & east_wall) == 0);
				}
				break;
			case left_is_clear:
				switch (facing){
					case facing_north:
						if (x == 0)
							return false;
						return ((world[x-1][y] & east_wall) == 0);
					case facing_east:
						return ((world[x][y] & north_wall) == 0);
					case facing_south:
						return ((world[x][y] & east_wall) == 0);
					case facing_west:
						if (y == 0)
							return false;
						return ((world[x][y-1] & north_wall) == 0);
				}
				break;
			case right_is_clear:
				switch (facing){
					case facing_north:
						return ((world[x][y] & east_wall) == 0);
					case facing_east:
						if (y == 0)
							return false;
						return ((world[x][y-1] & north_wall) == 0);
					case facing_south:
						if (x == 0)
							return false;
						return ((world[x-1][y ] & east_wall) == 0);
					case facing_west:
						return ((world[x][y] & north_wall) == 0);
				}
				break;
			case next_to_a_beeper:
				return ((world[x][y] >>> 2) > 0);
			case facing_north:
				return (facing == facing_north);
			case facing_east:
				return (facing == facing_east);
			case facing_south:
				return (facing == facing_south);
			case facing_west:
				return (facing == facing_west);
			case any_beepers_in_bag:
				return (bag > 0);
			case front_is_blocked:
				return ! test(front_is_clear);
			case left_is_blocked:
				return ! test(left_is_clear);
			case right_is_blocked:
				return ! test(right_is_clear);
			case not_next_to_a_beeper:
				return ((world[x][y] >>> 2) <= 0);
			case not_facing_north:
				return (facing != facing_north);
			case not_facing_east:
				return (facing != facing_east);
			case not_facing_south:
				return (facing != facing_south);
			case not_facing_west:
				return (facing != facing_west);
			case no_beepers_in_bag:
				return (bag <= 0);
		}
		return false;
	}

	static int exec_instr(int instr) {
		switch (instr) {
			case undef_instr:
				return incomplete_program_error;
			case move_instr:
				System.out.println("move");
				if (test(front_is_clear))
					switch (facing) {
						case facing_north:
							if (y >= world[0].length)
								return end_of_world_error;
							y++;
							return 0;
						case facing_south:
							if (y <= 0)
								return blocked_robot_error;
							y--;
							return 0;
						case facing_east:
							if (x >= world.length)
								return end_of_world_error;
							x++;
							return 0;
						case facing_west:
							if (x <= 0)
								return blocked_robot_error;
							x--;
							return 0;
						default:
							return internal_program_error;
					}
				else
					return blocked_robot_error;
			case turnleft_instr:
				System.out.println("turnleft");
				if (--facing < facing_north)
					facing = facing_west;
				return 0;
			case putbeeper_instr:
				System.out.println("putbeeper");
				if (bag <= 0)
					return empty_bag_error;
				bag--;
				world[x][y] += 4;
				return 0;
			case pickbeeper_instr:
				System.out.println("pickbeeper");
				if ((world[x][y] >>> 2) <= 0)
					return no_beeper_error;
				world[x][y] -= 4;
				bag++;
				return 0;
			case turnoff_instr:
				System.out.println("turnoff");
				return robot_made_turnoff;
		}
		return 0;
	}
}


class BackBuffer {
	static long buffer = 0;
	static int elements = 0;

	static void reset() {
		elements = 0;
	}

	static void push(boolean val) {
		if (elements < 64)
			elements++;
		buffer = buffer << 1;
		if (val)
			buffer += 1;
	}

	static boolean pop() {
		boolean retval;
		
		retval = (buffer & 1) != 0;
		if (elements > 0)
			elements--;
		buffer = buffer >>> 1;
		return retval;
	}
}


abstract class Node implements Globals{
	int  offset;
	Node next;

	Node() {
		offset = 0;
		next = null;
	}

	int length() {
		return 1;
	}

	abstract int description(int pos);

	int get_int_at_pos(int pos) {
		return 0;
	}

	Name get_name_at_pos(int pos){
		return null;
	}

	Node get_node_at_pos(int pos){
		return null;
	}

	void put_object_at_pos(int pos, int val) {
	}

	void put_object_at_pos(int pos, Name name) {
	}

	void put_object_at_pos(int pos, Node node) {
	}

	abstract void print(int level);
	abstract int exec_step();
}


final class ProgramNode extends Node implements Globals {

	static final int description[] =
		{program_node, is_def_list, is_execution, is_undef, is_undef};

	DefineNode    define;
	ExecutionNode execution;

	ProgramNode() {
		define = null;
		execution = new ExecutionNode();
		execution.next = this;
		execution.offset = 3;
	}

	int length() {
		return 2;
	}

	int description(int pos) {
		return description[pos];
	}

	Node get_node_at_pos(int pos){
		if (pos == 1)
			return define;
		else if (pos == 2)
			return execution;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 1)
			define = (DefineNode) node;
		else if (pos == 2)
			execution = (ExecutionNode) node;
	}

	void print(int level) {
		KarelTheRobot.println(level, "BEGINNING-OF-PROGRAM");
		if (define == null)
			KarelTheRobot.println(level + 1, "<definition>");
		else
			define.print(level + 1);
		execution.print(level + 1);
		KarelTheRobot.println(level, "END-OF-PROGRAM");
	}

	int exec_step() {
		if (KarelTheRobot.offset != 0)
			return missing_turnoff_error;
		if (execution == null)
			return incomplete_program_error;
		KarelTheRobot.instruction = execution;
		return 0;
	}
}


final class DefineNode extends Node implements Globals {

	static final int description[] =
		{define_node, is_name, is_stmt, is_undef, is_undef};

	Name name;
	Node instruction;

	static CallNode stack[] = new CallNode[1024];
	static int top = 0;

	DefineNode() {
		name = null;
		instruction = null;
	}

	static void reset() {
		top = 0;
	}

	int length() {
		return 2;
	}

	int description(int pos) {
		return description[pos];
	}

	Name get_name_at_pos(int pos){
		if (pos == 1)
			return name;
		return null;
	}

	void put_object_at_pos(int pos, Name name) {
		if (pos == 1)
			this.name = name;
	}

	Node get_node_at_pos(int pos){
		if (pos == 2)
			return instruction;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 2)
			instruction = node;
	}

	void print(int level) {
		KarelTheRobot.print(level, "DEFINE-NEW-INSTRUCTION ");
		if (name == null)
			KarelTheRobot.print(0, "<name>");
		else
			KarelTheRobot.print(0, name.get_name());
		KarelTheRobot.println(0, " AS");
		instruction.print(level + 1);
	}

	static int push(CallNode caller) {
		top++;
		if (top >= stack.length)
			return stack_overflow_error;
		stack[top] = caller;
		KarelTheRobot.instruction = caller.definition.define.instruction;
		return 0;
	}

	int exec_step() {
		if (KarelTheRobot.offset == 0)
			return internal_program_error;
		KarelTheRobot.offset = stack[top].offset;
		KarelTheRobot.instruction = stack[top].next;
		top--;
		return 0;
	}
}


final class Name implements Globals{

	private static Name name_list = null;

	String       name;
	DefineNode   define;
	private Name next;

	Name(String name, DefineNode define) {
		this.name = name;
		this.define = define;
		this.next = name_list;
		name_list = this;
	}

	public String get_name() {
		return this.name;
	}

	public static Name enter_name(String name, DefineNode definition) {
		Name nlist = name_list;
		while (nlist != null) {
			if (nlist.name.equals(name))
				if (nlist.define == null) {
					nlist.define = definition;
					return nlist;
				} else
					return null;
			nlist = nlist.next;
		}
		return new Name(name, definition);
	}

	public static DefineNode find_name(String name) {
		Name nlist = name_list;
		while (nlist != null) {
			if (nlist.name.equals(name))
				return nlist.define;
			nlist = nlist.next;
		}
		new Name(name, null);
		return null;
	}
}


final class ExecutionNode extends Node implements Globals {

	static final int description[] =
		{execution_node, is_stmt_list, is_undef, is_undef, is_undef};

	Node instruction;

	ExecutionNode() {
		instruction = null;
	}

	int description(int pos) {
		return description[pos];
	}

	Node get_node_at_pos(int pos){
		if (pos == 1)
			return instruction;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 1)
			instruction = node;
	}

	void print(int level) {
		KarelTheRobot.println(level, "BEGINNING-OF-EXECUTION");
		KarelTheRobot.print_instr(level, instruction);
		KarelTheRobot.println(level, "END-OF-EXECUTION");
	}

	int exec_step() {
		if (KarelTheRobot.offset == 0) {
			if (instruction == null)
				return incomplete_program_error;
			KarelTheRobot.instruction = instruction;
		} else {
			KarelTheRobot.offset = 0;
			KarelTheRobot.instruction = instruction;
		}
		return 0;
	}
}


final class BlockNode extends Node implements Globals {

	static final int description[] =
		{block_node, is_stmt_list, is_undef, is_undef, is_undef};

	Node instruction;

	BlockNode() {
		instruction = null;
	}

	int description(int pos) {
		return description[pos];
	}

	Node get_node_at_pos(int pos){
		if (pos == 1)
			return instruction;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 1)
			instruction = node;
	}

	void print(int level) {
		KarelTheRobot.println(level, "BEGIN");
		KarelTheRobot.print_instr(level, instruction);
		KarelTheRobot.println(level, "END");
	}

	int exec_step() {
		if (KarelTheRobot.offset == 0) {
			if (instruction == null)
				return incomplete_program_error;
			KarelTheRobot.instruction = instruction;
		} else {
			KarelTheRobot.offset = 0;
			KarelTheRobot.instruction = instruction;
		}
		return 0;
	}
}


final class WhileNode extends Node implements Globals {

	static final int description[] =
		{while_node, is_test, is_stmt, is_undef, is_undef};

	int  test;
	Node instruction;

	WhileNode() {
		test = undef_test;
		instruction = null;
	}

	int length() {
		return 2;
	}

	int description(int pos) {
		return description[pos];
	}

	int get_int_at_pos(int pos){
		if (pos == 1)
			return test;
		return 0;
	}

	void put_object_at_pos(int pos, int val) {
		if (pos == 1)
			test = val;
	}

	Node get_node_at_pos(int pos){
		if (pos == 2)
			return instruction;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 2)
			instruction = node;
	}

	void print(int level) {
		KarelTheRobot.println(level, "WHILE " + test_names[test] + " DO");
		KarelTheRobot.print_instr(level, instruction);
	}

	int exec_step() {
		if (test == 0)
			return incomplete_program_error;
		if (KarelTheRobot.offset == 0)
			BackBuffer.push(true);
		else
			BackBuffer.push(false);
		KarelTheRobot.offset = 0;
		if (KarelsWorld.test(test)) {
			if (instruction == null)
				return incomplete_program_error;
			KarelTheRobot.instruction = instruction;
		} else {
			KarelTheRobot.offset = offset;
			KarelTheRobot.instruction = next;
		}
		return 0;
	}
}


final class IterateNode extends Node implements Globals {

	static final int description[] =
		{iterate_node, is_number, is_stmt, is_undef, is_undef};

	int  count;
	Node instruction;
	
	static int stack[] = new int[1024];
	static int top = 0;

	IterateNode() {
		count = 0;
		instruction = null;
	}

	static void reset() {
		top = 0;
	}

	int length() {
		return 2;
	}

	int description(int pos) {
		return description[pos];
	}

	int get_int_at_pos(int pos){
		if (pos == 1)
			return count;
		return 0;
	}

	void put_object_at_pos(int pos, int val) {
		if (pos == 1)
			count = val;
	}

	Node get_node_at_pos(int pos){
		if (pos == 2)
			return instruction;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 2)
			instruction = node;
	}

	void print(int level) {
		if (count == 0)
			KarelTheRobot.println(level, "ITERATE <number> TIMES");
		else
			KarelTheRobot.println(level, "ITERATE " + count + " TIMES");
		KarelTheRobot.print_instr(level, instruction);
	}

	int exec_step() {
		if (KarelTheRobot.offset == 0) {
			if ((count == 0) || (instruction == null))
				return incomplete_program_error;
			top++;
			if (top >= stack.length)
				return stack_overflow_error;
			stack[top] = count;
			KarelTheRobot.instruction = instruction;
		} else {
			if (--stack[top] > 0) {
				KarelTheRobot.offset = 0;
				KarelTheRobot.instruction = instruction;
			} else {
				top--;
				KarelTheRobot.offset = offset;
				KarelTheRobot.instruction = next;
			}
		}
		return 0;
	}
}


class IfThenElseNode extends Node implements Globals {

	static final int description[] =
		{if_then_else_node, is_test, is_stmt, is_stmt, is_undef};

	int  test;
	Node then_stmt;
	Node else_stmt;

	IfThenElseNode() {
		test = undef_test;
		then_stmt = null;
		else_stmt = null;
	}

	int length() {
		return 3;
	}

	int description(int pos) {
		return description[pos];
	}

	int get_int_at_pos(int pos){
		if (pos == 1)
			return test;
		return 0;
	}

	void put_object_at_pos(int pos, int val) {
		if (pos == 1)
			test = val;
	}

	Node get_node_at_pos(int pos){
		if (pos == 2)
			return then_stmt;
		else if (pos == 3)
			return else_stmt;
		return null;
	}

	void put_object_at_pos(int pos, Node node) {
		if (pos == 2)
			then_stmt = node;
		else if (pos == 3)
			else_stmt = node;
	}

	void print(int level) {
		KarelTheRobot.println(level, "IF " + test_names[test]);
		KarelTheRobot.println(level, "THEN");
		KarelTheRobot.print_instr(level, then_stmt);
		KarelTheRobot.println(level, "ELSE");
		KarelTheRobot.print_instr(level, else_stmt);
	}

	int exec_step() {
		if (test == 0)
			return incomplete_program_error;
		if (KarelTheRobot.offset == 0) {
			if (KarelsWorld.test(test)) {
				if (then_stmt == null)
					return incomplete_program_error;
				KarelTheRobot.instruction = then_stmt;
			} else {
				if (else_stmt == null)
					return incomplete_program_error;
				KarelTheRobot.instruction = else_stmt;
			}
		} else {
			if (KarelTheRobot.offset == 4)
				BackBuffer.push(true);
			else
				BackBuffer.push(false);
			KarelTheRobot.offset = offset;
			KarelTheRobot.instruction = next;
		}
		return 0;
	}
}


final class CallNode extends Node implements Globals {

	static final int description[] =
		{call_node, is_name, is_undef, is_undef, is_undef};

	Name definition;

	CallNode() {
		definition = null;
	}

	int description(int pos) {
		return description[pos];
	}

	Name get_name_at_pos(int pos){
		if (pos == 1)
			return definition;
		return null;
	}

	void put_object_at_pos(int pos, Name name) {
		if (pos == 1)
			this.definition = name;
	}

	void print(int level) {
		KarelTheRobot.println(level, definition.get_name());
	}

	int exec_step() {
		if (KarelTheRobot.offset != 0)
			return internal_program_error;
		if ((definition == null) || (definition.define.instruction == null))
			return incomplete_program_error;
		return definition.define.push(this);
	}
}


final class BasicInstrNode extends Node implements Globals {

	static final int description[] =
		{basic_instr_node, is_instr, is_undef, is_undef, is_undef};

	int instruction;

	BasicInstrNode() {
		instruction = undef_instr;
	}

	BasicInstrNode(int instr) {
		instruction = instr;
	}

	int description(int pos) {
		return description[pos];
	}

	int get_int_at_pos(int pos){
		if (pos == 1)
			return instruction;
		return 0;
	}

	void put_object_at_pos(int pos, int val) {
		if (pos == 1)
			instruction = val;
	}

	void print(int level) {
		KarelTheRobot.println(level, instruction_names[instruction]);
	}

	int exec_step() {
		int retval;
		
		retval = KarelsWorld.exec_instr(instruction);
		KarelTheRobot.offset = offset;
		KarelTheRobot.instruction = next;
		return retval;
	}
}

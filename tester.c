#include <stdio.h>
#include <stdio.h> //
#include <stdlib.h> // 
#include "tester.h"

/*
 * How to approach this assignment.
 *
 * You need not maintain the O flag!
 *
 * 1. First, write each of the helper functions. Write them one
 * as a time and test them with the corresponding test cases.
 * You can test them using the main program we give you; this
 * will be much easier to debug.
 *
 * 2. Design an API that can call to execute a single instruction
 *	from the array of instructions you need to process.  Figure
 *	out what parameteres that function should take. Write the
 *	function and have it do nothing; add the right call to it
 *	in your main function. You are now ready to start building
 *	both your simulator and checker.
 *
 * 3. Write the y86_check function. Use the answer to question  Lab3-Ind.8
 *	to help you do this.
 * 4. Start building out your simulator (the function you designed in #2).
 *	Start by just having it return success.
 *	You should find that even this simple simulator passes the first test.
 *	(Once again, use the main program we give you.)
 * 5. Now, start building it out for real!
 * 	Write code to transform the string representation of an instruction
 *	to an enum (hint: look at the utility functions we provide for you;
 *	they are documented in tester.h).
 * 6. Now, start adding instructions incrementally.
 *	Follow the structure described on the main page for this assignment.
 *	The tests we give you in main.c also follow this structure.  Test
 * 	each category before moving on to the next.  Think about error cases!
 *	Every time you think of a kind of error you need to check for, encapsulate
 *	that check in a small function that you can easily test; then use it
 *	every time you need to make that test.
 *	Be careful: on an error, you must not have changed any state.
 *
 *	A note from Margo: I have been programming a long time. I redid this
 *	entire assignment after having already done it once earlier in the
 *	week; I had at least one bug to fix after every new thing I added.
 *	However, by testing each function and each instruction or each
 *	instruction class, the bugs were relatively easy to fix. Had I
 *	tried to do everything at the end, it would have taken two to
 *	three times longer (at least). I also used the main.c we give you
 *	to test entire sets before ever running the autograder.
 *	I also found that by building up lots of error functions, by the
 *	time I got to the last several classes, it was easy to assemble
 *	implementations for them.
 *
 *	While we provided a bunch of test cases, you are free to implement
 *	your own -- just copy a test case and edit it to do what you want!
 *	When you do run the autograder, if you fail a test, you will get
 *	output describing the test case. You can cut and paste the instructions
 *	and state directly into appropriate files in a new test case directory.
 *
 *	If you want to create your own tests, read tester.md.
 *
 * 7. When possible, identify helper functions you can write, e.g.,
 *	- Is there error checking that might be used by many instructions?
 *	- Can you think of functionality that might be shared across all
 *	  ALU ops?
 *	- What do the conditional jump and conditional move instructions
 *	  have in common?
 * 8. Finally, use the main program we give you to debug -- you can call functions
 *	from inside gdb and you will find this extraordinarily helpful, e.g.,
 *	call (void)dump_state(state)
 */

/* 
 * is_equal compares two y86 machine states for equivalence.
 * It returns 1 if s1 and s2 are equivalent.
 *
 * Unusual conditions: 
 * The memory states only need to match on valid bytes in memory.
 * The register states only need to match on valid y86 registers.
 * The flag bits only need to match on the specific flags supported
 * by the y86.

 assume that any addresses greater than or equal to start_addr + valid_mem are invalid
 (only check from start_addr to start_addr+valid_mem)
 */
int is_equal(y86_state_t *s1, y86_state_t *s2){
	if(s1->start_addr == s2->start_addr){
		//printf("start_addr match!\n");
		if(s1->valid_mem == s2->valid_mem){
			//printf("valid_mem match!\n");
			for(int i = 0; i <= s1->valid_mem; i++){
				if(s1->memory[i] != s2->memory[i]){
					return 0;
				}
			}
			//printf("memory match!\n");
			for(int j = 0x0; j < 0xf; j++){
				if(s1->registers[j] != s2->registers[j]){
					return 0;
					
				}
				
			}
			//printf("made it out of reg check");
			printf("PC1: %ld\n", s1->pc);
			printf("PC2: %ld\n", s2->pc);
			if(s1->pc == s2->pc){
				printf("PC match! \n");
				printf("flag1: %d\n", s1->flags);
				printf("flag2: %d\n", s2->flags);
				uint8_t overflow_flag1 = s1->flags & FLAG_O;
				uint8_t overflow_flag2 = s2->flags & FLAG_O;
				uint8_t zero_flag1 = s1->flags & FLAG_Z;
				uint8_t zero_flag2 = s2->flags & FLAG_Z;
				uint8_t signed_flag1 = s1->flags & FLAG_S;
				uint8_t signed_flag2 = s2->flags & FLAG_S;

				if((zero_flag1 == zero_flag2) && (overflow_flag1 == overflow_flag2) && (signed_flag1 == signed_flag2)){
					//printf("flags match! \n");
					return 1;
				}
			}
		}
	}
	return 0;
}

/*
 * read_quad reads the 8-byte value at 'address' from the memory of
 * the machine state 'state' and stores the result in 'value.'
 * It returns 1 if a read is successful and 0 if it fails.
 */
int read_quad(y86_state_t *state, uint64_t address, uint64_t *value) {
	int address_index = (address - state->start_addr);

	// printf("address: %lu \n", address);
	// printf("start_addr: %lu \n", state->start_addr);
	// printf("address_index: %d \n", address_index);
	// printf("valid mem: %ld \n", state->valid_mem);

	if(address_index <= (state->valid_mem - 8)){ //if the address is valid
		uint64_t hex_value = 0x0;
		hex_value = hex_value + (uint64_t) state->memory[address_index];
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 1] << 8);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 2] << 16);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 3] << 24);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 4] << 32);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 5] << 40);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 6] << 48);
		hex_value = hex_value + ((uint64_t) state->memory[address_index + 7] << 56);
		// printf("hex_value: %lX \n", hex_value);

		*value = hex_value;
		//*value = (uint64_t) state->memory[address_index];
		// printf("value: %ld \n", *value);
		
		return 1;
	}
	return 0;
}


/* 
 * write_quad writes the 8-byte item 'value' to the machine state at memory
 * address 'address'.
 * It returns 1 if a write is successful and 0 if it fails.
 */
int write_quad(y86_state_t *state, uint64_t address, uint64_t value) {
	int address_index = (address - state->start_addr);

	if(address_index <= (state->valid_mem - 8)){
		state->memory[address_index] = (value & 0xFF);
		//printf("index 0: %x\n", state->memory[address_index]);
		state->memory[address_index + 1] = ((value >> 8) & 0xFF);
		//printf("index 1: %x\n", state->memory[address_index + 1]);
		state->memory[address_index + 2] = ((value >> 16) & 0xFF);
		state->memory[address_index + 3] = ((value >> 24) & 0xFF);
		state->memory[address_index + 4] = ((value >> 32) & 0xFF);
		state->memory[address_index + 5] = ((value >> 40) & 0xFF);
		state->memory[address_index + 6] = ((value >> 48) & 0xFF);
		state->memory[address_index + 7] = ((value >> 56) & 0xFF);

		return 1;
	}

	return 0;

	/*
	address_index is the index in state->memory that the first byte needs to be written at
	check address is valid (has at least 8 bytes of valid mem left)
	*/
}

	//sets the signed flag to 1 if set_true is 1, sets to 0 if not.
void set_signed_flag(y86_state_t *state, int set_true){
	if(set_true){
		state->flags |= 1 << 7; 
	}else{
		state->flags &= ~(1 << 7);
	}
}

void set_zero_flag(y86_state_t *state, int set_true){
	if(set_true){
		state->flags |= 1 << 6; 
	}else{
		state->flags &= ~(1 << 6);
	}
}


void set_flags(y86_state_t *state, int64_t value, y86_inst_t curr_inst){
	//printf("VALUE: %ld\n", value);
	if(value < 0){
		//printf("SWAG1\n");
		set_signed_flag(state, 1);
		set_zero_flag(state, 0);
	}else if(value == 0){
		//printf("SWAG2\n");
		set_signed_flag(state, 0);
		set_zero_flag(state, 1);
	}else if(value > 0){
		//printf("SWAG3\n");
		set_signed_flag(state, 0);
		set_zero_flag(state, 0);	
	}
}
int check_reg(uint8_t r){
	if((r > 15) || (r < 0)){
		return 0;
	}
	return 1;
}

/*
simulator function. takes a state and series of instructions, does n_inst instructions.
return 1 if it executed the instructions successfully, return 0 otherwise.
*/
int y86_func(y86_state_t *state, y86_inst_t *instructions, int n_inst){
	
	for(int i = 0; i < n_inst; i++){
		y86_inst_t curr_inst = instructions[i];
		enum _inst_t curr_inst_enum = inst_to_enum(curr_inst.instruction);
		//printf("got here\n");
		if(curr_inst_enum == I_NOP){
			
			//printf("flags: %d\n", state->flags);
			state->pc = state->pc + 1;
			
		}else if(curr_inst_enum == I_IRMOVQ){
			if(check_reg(curr_inst.rB)){
				state->registers[curr_inst.rB] = curr_inst.constval;
				state->pc = state->pc + 10;
			}else{
				return 0;
			}
			
		}else if(curr_inst_enum == I_RRMOVQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_ADDQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				

				//sim_good(state, instructions, n_inst);
				
				int64_t rA_value =  state->registers[curr_inst.rA];
				int64_t rB_value = state->registers[curr_inst.rB];
				
				
				set_flags(state, (rA_value + rB_value), curr_inst);

				state->registers[curr_inst.rB] = rB_value + rA_value;
				
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
			
		}
		else if(curr_inst_enum == I_SUBQ){
			//dump_state(state);
			// sim_good(state, instructions, n_inst);
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);

				set_flags(state, (long int)state->registers[curr_inst.rB] - (long int) state->registers[curr_inst.rA], curr_inst);
				state->registers[curr_inst.rB] = (long int)state->registers[curr_inst.rB] - (long int) state->registers[curr_inst.rA];
				
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_XORQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				set_flags(state, (state->registers[curr_inst.rB] ^ state->registers[curr_inst.rA]), curr_inst);
				state->registers[curr_inst.rB] = (state->registers[curr_inst.rB] ^ state->registers[curr_inst.rA]);
				state->pc = state->pc + 2;
			}else{
				return 0;
			}

		}else if(curr_inst_enum == I_ANDQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				set_flags(state, (state->registers[curr_inst.rB] & state->registers[curr_inst.rA]), curr_inst);
				state->registers[curr_inst.rB] = (state->registers[curr_inst.rB] & state->registers[curr_inst.rA]);
				state->pc = state->pc + 2;
			}else{
				return 0;
			}

		}else if(curr_inst_enum == I_MULQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				set_flags(state, (state->registers[curr_inst.rB] * state->registers[curr_inst.rA]), curr_inst);
				state->registers[curr_inst.rB] = (state->registers[curr_inst.rB] * state->registers[curr_inst.rA]);
				state->pc = state->pc + 2;
			}else{
				return 0;
			}

		}else if(curr_inst_enum == I_DIVQ){
			//sim_good(state, instructions, n_inst);
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA) && (state->registers[curr_inst.rA] != 0)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				set_flags(state, (state->registers[curr_inst.rB] / state->registers[curr_inst.rA]), curr_inst);
				state->registers[curr_inst.rB] = (state->registers[curr_inst.rB] / state->registers[curr_inst.rA]);
				state->pc = state->pc + 2;
			}else{
				return 0;
			}

		}else if(curr_inst_enum == I_MODQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA) && (state->registers[curr_inst.rA] != 0)){
				set_zero_flag(state, 0);
				set_signed_flag(state, 0);
				set_flags(state, (state->registers[curr_inst.rB] % state->registers[curr_inst.rA]), curr_inst);
				state->registers[curr_inst.rB] = (state->registers[curr_inst.rB] % state->registers[curr_inst.rA]);
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_CMOVEQ){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if((state->flags & 0x40) == 0x40){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_CMOVNE){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if((state->flags & FLAG_Z) != FLAG_Z){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_CMOVL){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if((state->flags & FLAG_S) == FLAG_S){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_CMOVLE){
			// dump_state(state);
			// printf("rB: %d\n", curr_inst.rB);
			// printf("rA: %d\n", curr_inst.rA);
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if(((state->flags & FLAG_Z) == FLAG_Z) || ((state->flags & FLAG_S) == FLAG_S)){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
			// dump_state(state);
		}else if(curr_inst_enum == I_CMOVG){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if(((state->flags & FLAG_S) != FLAG_S) && ((state->flags & FLAG_Z) != FLAG_Z)){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_CMOVGE){
			if(check_reg(curr_inst.rB) && check_reg(curr_inst.rA)){
				if((state->flags & FLAG_S) != FLAG_S){
					state->registers[curr_inst.rB] = state->registers[curr_inst.rA];
				}
				state->pc = state->pc + 2;
			}else{
				return 0;
			}
		}else if(curr_inst_enum == I_J){
			dump_state(state);
			//sim_good(state, instructions, n_inst);
			state->pc = curr_inst.constval;
			dump_state(state);

		}else if(curr_inst_enum == I_JL){
			if((state->flags & FLAG_S) == FLAG_S){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JL){
			if((state->flags & FLAG_S) == FLAG_S){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JLE){
			if(((state->flags & FLAG_S) == FLAG_S) || ((state->flags & FLAG_Z) == FLAG_Z)){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JNE){
			if((state->flags & FLAG_Z) != FLAG_Z){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JEQ){
			if((state->flags & FLAG_Z) == FLAG_Z){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JGE){
			if((state->flags & FLAG_S) != FLAG_S){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}else if(curr_inst_enum == I_JG){
			if(((state->flags & FLAG_Z) != FLAG_Z) && ((state->flags & FLAG_S) != FLAG_S)){
				state->pc = 0;
			}else{
				state->pc = state->pc + 9;
			}
			
		}
		else if(curr_inst_enum == I_MRMOVQ){
			dump_state(state);
			//sim_good(state, instructions, n_inst);
			

			if((curr_inst.constval + curr_inst.rA) > (state->start_addr + state->valid_mem)){
				return 0;
			}else{
				uint64_t *value = malloc(sizeof(uint64_t));
				read_quad(state, curr_inst.rB, value); 

				
				state->registers[curr_inst.rA] = *value;
				state->pc = state->pc + 10;
				free(value);
			}
			dump_state(state);
		}
		
		
	}
	
		
		return 1;

	}

/*
 * y86_check returns 0 if the y86sim_func properly simulates
 * the n_inst instructions described in the instructions array, and
 * non-zero otherwise.
 *
 * Hint: To validate that 'simfunc' executed properly, you will need
 * to produce the correct end state. That means that you must write
 * your own simulator to produce that correct end state.
 *
 * Unusual conditions:
 * On halt: stop execution, the state should reflect the last executed
 * 	instruction.
 * On an invalid command: stop execution, the state should reflect the last
 * 	executed instruction.
 * On any bad arguments, e.g., invalid register number or access to an
 *	invalid address, divide by 0, stop execution, the state should
 * 	reflect the last executed instruction
 */
int y86_check(y86_state_t *state, y86_inst_t *instructions, int n_inst,
    y86sim_func simfunc) {

		//y86_state_t state_copy = *state;
		y86_state_t *state_copy_ptr = malloc(sizeof(y86_state_t));
		*state_copy_ptr = *state;

		int sim_pass = y86_func(state, instructions, n_inst);
		simfunc(state_copy_ptr, instructions, n_inst);
		if(is_equal(state, state_copy_ptr) && sim_pass){
			free(state_copy_ptr);
			return 0;
		}
		free(state_copy_ptr);
		return -1;
}

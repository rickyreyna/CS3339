/******************************
 * Name:  Ricardo Reyna rjr110
 * CS 3339 - Spring 2019
 ******************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();
    mem();
    writeback();
    stats.clock();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  opcode = (instr >> 26);
  rs = (instr >> 21) & 0x1f;
  rt = (instr >> 16) & 0x1f;
  rd = (instr >> 11) & 0x1f;
  shamt = (instr >> 6) & 0x1f;
  funct = instr & 0x3f;
  uimm = instr & 0xffff;
  simm = ((signed) uimm << 16) >> 16;
  addr = instr & 0x3ffffff;

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!

  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  writeDest = false;
  aluOp = ADD;
  aluSrc1 = regFile[REG_ZERO];
  aluSrc2 = regFile[REG_ZERO];
  destReg = regFile[REG_ZERO];
  storeData = 0;

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");

  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt); ///R Instruction - regFile[rd] = regFile[rt] << shamt;
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = shamt;
                   break;
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt); ///I Instruction - regFile[rd] = regFile[rt] >> shamt;
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = shamt;
                   break;
        case 0x08: D(cout << "jr " << regNames[rs]); ///R Instruction - pc = regFile[rs];
                   stats.registerDest(REG_ZERO);
                   pc = regFile[rs]; stats.registerSrc(rs);
                   stats.flush(2);
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]); ///R Instruction - regFile[rd] = hi;
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = ADD;
                   aluSrc1 = hi; stats.registerSrc(REG_HILO);
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]); ///R Instruction - regFile[rd] = lo;
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = ADD;
                   aluSrc1 = lo; stats.registerSrc(REG_HILO);
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]); ///R Instruction
                   opIsMultDiv = true; stats.registerDest(REG_HILO);
                   aluOp = MUL;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]); ///R Instruction - lo = regFile[rs] / regFile[rt];  hi = regFile[rs] % regFile[rt];
                   opIsMultDiv = true; stats.registerDest(REG_HILO);
                   aluOp = DIV;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); ///R Instruction - regFile[rd] = regFile[rs] + regFile[rt];
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); ///R Instruction - regFile[rd] = regFile[rs] - regFile[rt];
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = -regFile[rt]; stats.registerSrc(rt);
                   break;
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); ///R Instruction
                   writeDest = true; destReg = rd; stats.registerDest(rd);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt);
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4 //J Instruction
               stats.registerDest(REG_ZERO);
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               break;
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4 //J Instruction
               writeDest = true; destReg = REG_RA; /* writes PC+4 to $ra */ stats.registerDest(REG_RA);
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; stats.registerSrc(REG_ZERO); // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               break;
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2)); ///I Instruction
               stats.registerDest(REG_ZERO);
               stats.registerSrc(rs); stats.registerSrc(rt);
               stats.countBranch();
               if(regFile[rs] == regFile[rt])
               {
                   pc = pc + (simm << 2);
                   stats.countTaken();
                   stats.flush(2);
               }
               break;  // read the handout carefully, update PC directly here as in jal example
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2)); ///I Instruction
               stats.registerDest(REG_ZERO);
               stats.registerSrc(rs); stats.registerSrc(rt);
               stats.countBranch();
               if(regFile[rs] != regFile[rt])
               {
                   pc = pc + (simm << 2);
                   stats.countTaken();
                   stats.flush(2);
               }
               break;  // same comment as beq
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm); ///I Instruction - regFile[rt] = regFile[rs] + simm;
               writeDest = true; destReg = rt; stats.registerDest(rt);
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs);
               aluSrc2 = simm;
               break;
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm); ///I Instruction
               writeDest = true; destReg = rt; stats.registerDest(rt);
               aluOp = AND;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs);
               aluSrc2 = uimm;
               break;
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm); ///I Instruction
               writeDest = true; destReg = rt; stats.registerDest(rt);
               aluOp = SHF_L;
               aluSrc1 = simm; stats.registerSrc(REG_ZERO);
               aluSrc2 = 16;
               break;
    case 0x1a: D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; stats.registerSrc(rs);
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; stats.registerDest(rt);
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")"); ///I Instruction
               writeDest = true; destReg = rt; stats.registerDest(rt);
               opIsLoad = true; stats.countMemOp();
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs);
               aluSrc2 = simm;
               break;  // do not interact with memory here - setup control signals for mem() below
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")"); ///I Instruction
               writeDest = false; destReg = REG_ZERO; stats.registerDest(REG_ZERO);
               opIsStore = true; stats.countMemOp();
               storeData = regFile[rt];
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rt); stats.registerSrc(rs); /// Toona says it uses both registers sources
               aluSrc2 = simm;
               break;  // same comment as lw
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad)
    writeData = dMem.loadWord(aluOut);
  else
    writeData = aluOut;

  if(opIsStore)
    dMem.storeWord(storeData, aluOut);
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip if write is to zero register
    regFile[destReg] = writeData;

  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl << endl;

  cout << "Cycles: " << stats.getCycles() << endl;
  cout << "CPI: " << fixed << setprecision(2) << 1.0 * (stats.getCycles() / static_cast<double>(instructions)) << endl << endl;

  cout << "Bubbles: " << stats.getBubbles() << endl;
  cout << "Flushes: " << stats.getFlushes() << endl << endl;

  cout << "Mem ops: " << fixed << setprecision(1) << ((stats.getMemOps() * 100.0) / instructions) <<  "% of instructions" << endl;
  cout << "Branches: " << fixed << setprecision(1) << ((stats.getBranches() * 100.0) / instructions) <<  "% of instructions" << endl;
  cout << "  % Taken: " << fixed << setprecision(1) << ((stats.getTaken() * 100.0) / stats.getBranches()) << endl;
}

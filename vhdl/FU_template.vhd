library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";

entity FU is
    generic (
                INSTRUCTIONS : natural;
                TOTAL_EXE_CYCLES : natural;
                BITWIDTH : natural; -- 31 -> meaning 32
                RF_DEPTH: natural; --15 -> meaning 16
                INPUT_PORTS : natural;
                OUTPUT_PORTS : natural;
                OPCODE :natural; -- 0 -> ADD, 1 -> MUL
                IS_MUL: natural -- Set to 1 only if OPCODE is MUL, 0 otherwise
                                -- This takes into account the fact that the
                                -- MUL output is 2 times the input BITWIDTH
            );
    port(
        i_clock : in std_logic;
        --specify inputs (number of FUs that sends input to this FU)
        --i_FU : in std_logic_vector(natural(real(IS_MUL)/real(2)*real(BITWIDTH +1)*real(INPUT_PORTS))-1 downto 0);
        i_FU : in std_logic_vector(((BITWIDTH +1)*INPUT_PORTS)-
        IS_MUL*((BITWIDTH +1)/2*INPUT_PORTS)   -1
        downto 0);
        --i_FU1 : in signed(32 downto 0);
        --i_FU2 : in signed(32 downto 0);
        --i_FU3 : in signed(32 downto 0);
        --i_FU4 : in signed(32 downto 0);
        --specify outputs (number of FUs that this FU can send the output to)
        o_FU : out std_logic_vector(((BITWIDTH +1)*OUTPUT_PORTS)-1 downto 0);
        --o_FU1 : out signed(32 downto 0);
        --o_FU2 : out signed(32 downto 0);
        --o_FU3 : out signed(32 downto 0)
    -- SIGNALS FOR INSTRUCTION MEMORY
        r_COMPUTING : in std_logic;
        r_LOAD_INST: in std_logic := '0';
        r_LOAD_NEXT_INST: in std_logic ;
        r_INPUT_INST : in std_logic_vector(
                natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ -- bit required to ID Clock
                natural(ceil(log2(real(INPUT_PORTS))))*3 + -- crossbar Select Bits
                2 + -- Register Write enable bits (input 1 and input 2)
                natural(ceil(log2(real(RF_DEPTH+1)))) * 4 + -- RF select bits
                4 -- Select signals for MuxA and MuxB
                -1
                downto 0);

        r_RESET : std_logic
    -- SIGNALS FOR REGISTER FILE

    );
end FU;

architecture rtl of FU is
    --Parametrize the total number of cycles
    --Cunter that keeps track of the current cycle number
    signal instruction_count: natural range 0 to TOTAL_EXE_CYCLES;

    --The select signal for the input crossbar needs to be log_2(#inputs)*#outputs
    -- in this case log_2(4)*3 = 6
    constant crossbarSelectBITs : natural := natural(ceil(log2(real(INPUT_PORTS))));
    --signal input_crossbar_select : std_logic_vector((3*(crossbarSelectBITs))-1 downto 0);
    --type inputCrossbariType is array ( 0 to INPUT_PORTS-1) of std_logic_vector(BITWIDTH downto 0);
    --signal inputCrossbar : inputCrossbarType;
    -- The input crossbar has 3 outputs : MuxA, MuxB, RFs

    constant INPUT_BITWIDTH : natural :=  ((BITWIDTH +1) - IS_MUL*(BITWIDTH + 1)/2) -1;

    constant RFSelectBits : natural := natural(ceil(log2(real(RF_DEPTH+1))));
          -- natural(ceil(log2(real(INPUT_PORTS)))) * 3 + 2 Bits for reg write enable +
          -- + natural(ceil(log2(real(RF_DEPTH)))) *4 + 2 bits MuxA + 2 bits MuxB

    constant INSTRUCTION_SIZE :natural := crossbarSelectBITs * 3 + 2 + RFSelectBits *4 + 4;
    signal current_instruction: natural range 0 to INSTRUCTION_SIZE;
    constant CycleBitsSize  : natural := natural(ceil(log2(real(TOTAL_EXE_CYCLES))));

    -- SIGNALS FOR INSTRUCTION MEMORY
    signal w_OUTPUT_INST : std_logic_vector(INSTRUCTION_SIZE-1 downto 0); -- INSTRUCTION_SIZE
    signal w_VALID_INST : std_logic := '0';
    -- UNDER TEST
signal muxAout : std_logic_vector(BITWIDTH downto 0);
signal muxBout : std_logic_vector(BITWIDTH downto 0);
    --END UNDER TEST

    -- SIGNALS FOR REGISTER FILE
    signal w_OUTPUT_A : std_logic_vector(BITWIDTH downto 0);
    signal w_OUTPUT_B : std_logic_vector(BITWIDTH downto 0);
    signal r_INPUT_1 : std_logic_vector(BITWIDTH downto 0);
    signal r_INPUT_2 : std_logic_vector(BITWIDTH downto 0);
    -- SIGNAL FOR FU OUTPUT
    signal opOUT: std_logic_vector(BITWIDTH downto 0) := (OTHERS => '0');
    signal RESULT: std_logic_vector(BITWIDTH downto 0) := (OTHERS => '0');

    -- DEBUG SIGNALS
    signal muxa_in1 :std_logic_vector(BITWIDTH downto 0);
    signal muxa_in2 :std_logic_vector(BITWIDTH downto 0);
    signal muxa_in3 :std_logic_vector(BITWIDTH downto 0);
    signal muxb_in1 :std_logic_vector(BITWIDTH downto 0);
    signal muxb_in2 :std_logic_vector(BITWIDTH downto 0);
    signal muxb_in3 :std_logic_vector(BITWIDTH downto 0);
    -- END DEBUG SIGNALS
        --BEGIN DEBUG SIGNALS
    signal w_SEL_MUXA : std_logic_vector(1 downto 0);
    signal w_SEL_MUXB : std_logic_vector(1 downto 0);
    signal w_MUXA_OUT : std_logic_vector(BITWIDTH downto 0);
    signal w_MUXB_OUT : std_logic_vector(BITWIDTH downto 0);
    signal d_in_cb_out1_sel : natural range 0 to INPUT_PORTS -1;
    signal d_in_cb_out2_sel : natural range 0 to INPUT_PORTS -1;
    signal d_in_cb_out3_sel : natural range 0 to INPUT_PORTS -1;
    signal d_input_crossbar_out1 : std_logic_vector(INPUT_BITWIDTH downto 0);
    signal d_input_crossbar_out2 : std_logic_vector(INPUT_BITWIDTH downto 0);
    signal d_input_crossbar_out3 : std_logic_vector(INPUT_BITWIDTH downto 0);
        --END DEBUG SIGNALS
    component instruction_memory is
    generic (MAX_INSTRUCTION : natural;
             INSTRUCTION_SIZE : natural;
             TOT_NUM_CYCLES : natural);
        port
        (
        clock : in std_logic;
        computing : in std_logic;
        load_inst : in std_logic;
        load_next_inst : in std_logic;
        -- len of input inst
        -- 2 bits for MuxA
        -- 2 bits for MuxB
        -- 2 bits for Demux after OP
        -- 2 bits for Demux Output ports (if we output on all ports not required)
        -- 4 bits for Reg1SelectWrite
        -- 4 bits for Reg2SelectWrite
        -- 4 bits for Reg1SelectRead
        -- 4 bits for Reg2SelectRead
        -- 1 bit for RegInput1WriteEn
        -- 1 bit for RegInput2WriteEn
        -- 6 bits for inputCrossbarSelect
        -- 8 bits for clock (256 TOT_NUM_CYCLE) log_2(TOT_NUM_CYCLE) to identify clock
        ---------------------------------------
        -- 40 bits total to store
        input_inst : in std_logic_vector(natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ INSTRUCTION_SIZE-1 downto 0);
        -- The actual inst size is 40 - 8 (bit for clock) 32 bits
        output_inst : out std_logic_vector(INSTRUCTION_SIZE-1 downto 0); -- INSTRUCTION_SIZE
        valid_inst : out std_logic :='0';
        reset : in std_logic
        );
    end component instruction_memory;

    component register_file is
        generic
        (
            BITWIDTH : natural; --31
            DEPTH: natural -- 15
        );
        port
        (
            outA: out std_logic_vector(BITWIDTH downto 0);
            outB: out std_logic_vector(BITWIDTH downto 0);
            input1: in std_logic_vector(BITWIDTH downto 0);
            input2: in std_logic_vector(BITWIDTH downto 0);
            writeEnable1: in std_logic;
            writeEnable2: in std_logic;

            --the select signal needs to be log2(registerfilesize)
            reg1Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            reg2Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            reg1Sel_write : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            reg2Sel_write : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
            clock : in std_logic;
            reset : in std_logic
        );
    end component register_file;

begin
    --Instantiate the Unit Under Test (UUT)
    IM : instruction_memory
        generic map (
            MAX_INSTRUCTION => INSTRUCTIONS,
          -- natural(ceil(log2(real(INPUT_PORTS)))) * 3 + 2 Bits for reg write enable +
          -- + natural(ceil(log2(real(RF_DEPTH)))) *4 + 2 bits MuxA + 2 bits MuxB
            INSTRUCTION_SIZE => INSTRUCTION_SIZE,
            TOT_NUM_CYCLES => TOTAL_EXE_CYCLES
        ) -- meaning from 0 to 7 included (8 total)
        port map (
        clock => i_clock,
        computing => r_COMPUTING ,
        load_inst => r_LOAD_INST ,
        load_next_inst => r_LOAD_NEXT_INST ,
        -- len of input inst
        -- 2 bits for MuxA
        -- 2 bits for MuxB
        -- 2 bits for Demux after OP
        -- 2 bits for Demux Output ports (if we output on all ports not required)
        -- 4 bits for Reg1SelectWrite
        -- 4 bits for Reg2SelectWrite
        -- 4 bits for Reg1SelectRead
        -- 4 bits for Reg2SelectRead
        -- 1 bit for RegInput1WriteEn
        -- 1 bit for RegInput2WriteEn
        -- 6 bits for inputCrossbarSelect
        -- 8 bits for clock (256 TOT_NUM_CYCLE) log_2(TOT_NUM_CYCLE) to identify clock
        ---------------------------------------
        -- 50 bits total to store
        input_inst => r_INPUT_INST ,
        -- The actual inst size is 50 - 8 (bit for clock) 32 bits
        output_inst => w_OUTPUT_INST ,
        valid_inst => w_VALID_INST ,
        reset => r_RESET
                 );

    RF : register_file
        generic map (
            BITWIDTH => BITWIDTH,
            DEPTH => RF_DEPTH
                    )
        port map (
        outA => w_OUTPUT_A,
        outB => w_OUTPUT_B,
        input1 => r_INPUT_1,
        input2 => r_INPUT_2,
        --writeEnable1 => w_OUTPUT_INST(31-6),
        writeEnable1 => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs *3),
        --writeEnable2 => w_OUTPUT_INST(31-7),
        writeEnable2 => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs *3-1),

        --the select signal needs to be log2(registerfilesize)
        --reg1Sel_read  => w_OUTPUT_INST(31-8 downto 31-8-4+1),
        reg1Sel_read  => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2
                            downto INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - RFSelectBits +1),
        --reg2Sel_read  => w_OUTPUT_INST(31-8-4 downto 31-8-8+1),
        reg2Sel_read  => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - RFSelectBits
                            downto INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - 2*RFSelectBits +1),
        --reg1Sel_write  => w_OUTPUT_INST(31-8-8 downto 31-8-8-4+1),
        reg1Sel_write  => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - 2*RFSelectBits
        downto INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - 3*RFSelectBits+1),
        --reg2Sel_write  => w_OUTPUT_INST(31-8-12 downto 31-8-12-4+1),
        reg2Sel_write  => w_OUTPUT_INST(INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - 3*RFSelectBits
        downto INSTRUCTION_SIZE-1 -crossbarSelectBITs*3-2 - 4*RFSelectBits+1),
        clock => i_clock,
        reset  => r_RESET
                 );

    --genCrossbar: for i in 0 to INPUT_PORTS -1 generate
    --    inputCrossbar(i) <= i_FU(((i+1) * (BITWIDTH+1))-1 downto (i*(BITWIDTH+1)));
    --end generate;
   -- input_crossbar_out1 <= inputCrossbar(inputCrossbarSelect((crossbarSelectBITs)-1 downto 0));
    --input_crossbar_out2 <= inputCrossbar(inputCrossbarSelect((2*(crossbarSelectBITs))-1 downto ((crossbarSelectBITs)-1)));
    --input_crossbar_out3 <= inputCrossbar(inputCrossbarSelect((3*(crossbarSelectBITs))-1 downto ((2*(crossbarSelectBITs))-1)));
    --genOutputPorts: for i in 0 to (OUTPUT_PORTS-1) generate
    --    genOutput: for j in 0 to (BITWIDTH) generate
    --        o_FU((i*(BITWIDTH))+j) <= opOut(j);
    --    end generate genOutput;
    --end generate genOutputPorts;

    genOutputPorts: for i in 0 to (OUTPUT_PORTS-1) generate
            o_FU(((i+1)*(BITWIDTH+1)-1) downto (i*(BITWIDTH+1))) <= opOut;
    end generate genOutputPorts;

    ADDOP: if OPCODE = 0 generate
            RESULT <= std_logic_vector(unsigned(muxAout) + unsigned(muxBout));
    end generate;

    MULOP: if OPCODE = 1 generate
            RESULT <= std_logic_vector(unsigned(muxAout(((BITWIDTH+1)/2)-1 downto 0)) * unsigned(muxBout(((BITWIDTH+1)/2)-1 downto 0)));
    end generate;

    -- not sure if this should be here, well... if it is not here it does not work, so I guess it should be here
    r_INPUT_2 <= opOut;
    fuProc : process(i_clock ) is
        variable in_cb_out1_sel : natural range 0 to INPUT_PORTS -1;
        variable in_cb_out2_sel : natural range 0 to INPUT_PORTS -1;
        variable in_cb_out3_sel : natural range 0 to INPUT_PORTS -1;
        variable input_crossbar_out1 : std_logic_vector(INPUT_BITWIDTH downto 0);
        variable input_crossbar_out2 : std_logic_vector(INPUT_BITWIDTH downto 0);
        variable input_crossbar_out3 : std_logic_vector(INPUT_BITWIDTH downto 0);
        variable inputCrossbarSelect : std_logic_vector((3*(crossbarSelectBITs))-1 downto 0);
        --variable muxAout : std_logic_vector(BITWIDTH downto 0);
        --variable muxBout : std_logic_vector(BITWIDTH downto 0);
        variable muxAoutSel : std_logic_vector(1 downto 0);
        variable muxBoutSel : std_logic_vector(1 downto 0);

    begin
        --if rising_edge(i_clock) then
            if r_COMPUTING = '1' then
                if w_VALID_INST = '1' then
                    --here the number of sel bits for the input crossbar is hardcoded (6)
                    --need to parametrize also in instruction memory
                   inputCrossbarSelect := w_OUTPUT_INST(INSTRUCTION_SIZE-1 downto (INSTRUCTION_SIZE-1 - (crossbarSelectBITs*3 -1 )));
                   in_cb_out3_sel := to_integer(unsigned(inputCrossbarSelect((crossbarSelectBITs)-1 downto 0)));
                   d_in_cb_out3_sel <= in_cb_out3_sel;
                   input_crossbar_out3 := i_FU((((INPUT_PORTS-1)-in_cb_out3_sel +1) * (INPUT_BITWIDTH +1))-1 downto (((INPUT_PORTS-1)-in_cb_out3_sel) * (INPUT_BITWIDTH+1)));
                   d_input_crossbar_out3 <= input_crossbar_out3;

                   in_cb_out2_sel := to_integer(unsigned(inputCrossbarSelect((2*(crossbarSelectBITs))-1 downto ((crossbarSelectBITs)))));
                   d_in_cb_out2_sel <= in_cb_out2_sel;
                   input_crossbar_out2 := i_FU( (((INPUT_PORTS-1)-in_cb_out2_sel +1) * (INPUT_BITWIDTH +1))-1 downto (((INPUT_PORTS-1)-in_cb_out2_sel) * (INPUT_BITWIDTH+1)));
                   d_input_crossbar_out2<= input_crossbar_out2;

                   in_cb_out1_sel := to_integer(unsigned(inputCrossbarSelect((3*(crossbarSelectBITs))-1 downto ((2*(crossbarSelectBITs))))));
                   d_in_cb_out1_sel<= in_cb_out1_sel;
                   input_crossbar_out1 := i_FU( (((INPUT_PORTS-1)-in_cb_out1_sel +1) * (INPUT_BITWIDTH +1))-1 downto (((INPUT_PORTS-1)-in_cb_out1_sel) * (INPUT_BITWIDTH+1)));
                   d_input_crossbar_out1 <= input_crossbar_out1;

                   muxAoutSel := w_OUTPUT_INST(1 downto 0);
                   w_SEL_MUXA <= w_OUTPUT_INST(1 downto 0);
                   muxBoutSel := w_OUTPUT_INST(3 downto 2);
                   w_SEL_MUXB <= w_OUTPUT_INST(3 downto 2);

                   -- opOut <= std_logic_vector(unsigned(muxAout) + unsigned(muxBout));
                   opOut <= RESULT;
                   -- DEBUG MUX A
                    muxa_in1 <= opOut;
                    muxa_in2(INPUT_BITWIDTH downto 0) <= input_crossbar_out1;
                    muxa_in3 <= w_OUTPUT_A;
                   -- END DEBUG MUX A
                   --case muxAoutSel is
                   --    when "00" => muxAout := opOut;
                   --    when "01" => muxAout := input_crossbar_out1;
                   --    when "10" => muxAout := w_OUTPUT_A;
                   --    when others => muxAout := (OTHERS=>'0');
                   --end case;
                   case muxAoutSel is
                       when "00" => muxAout <= opOut;
                       when "01" => muxAout(INPUT_BITWIDTH downto 0) <= input_crossbar_out1;
                       when "10" => muxAout <= w_OUTPUT_A;
                       when others => muxAout <= (OTHERS=>'0');
                   end case;
                   w_MUXA_OUT <= muxAout;
                   -- DEBUG MUX B
                    muxb_in1 <= opOut;
                    muxb_in2(INPUT_BITWIDTH downto 0) <= input_crossbar_out2;
                    muxb_in3 <= w_OUTPUT_B;
                   -- END DEBUG MUX B

                   --case muxBoutSel is
                   --    when "00" => muxBout := opOut;
                   --    when "01" => muxBout := input_crossbar_out2;
                   --    when "10" => muxBout := w_OUTPUT_B;
                   --    when others => muxBout := (OTHERS=>'0');
                   --end case;

                   case muxBoutSel is
                       when "00" => muxBout <= opOut;
                       when "01" => muxBout(INPUT_BITWIDTH downto 0) <= input_crossbar_out2;
                       when "10" => muxBout <= w_OUTPUT_B;
                       when others => muxBout <= (OTHERS=>'0');
                   end case;
                   w_MUXB_OUT <= muxBout;
                   r_INPUT_1(INPUT_BITWIDTH downto 0) <= input_crossbar_out3;


                end if;
            end if;
        --end if;

    end process;
end rtl;

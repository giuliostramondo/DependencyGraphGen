library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity FU_instruction_memory_tb is
end FU_instruction_memory_tb;

architecture behave of FU_instruction_memory_tb is
    constant c_CLOCK_PERIOD : time := 50 ns;
    signal r_CLOCK : std_logic := '0';
    signal r_COMPUTING : std_logic := '0';
    signal r_LOAD_INST: std_logic := '0';
    signal r_LOAD_NEXT_INST: std_logic := '0';
    signal r_INPUT_INST : std_logic_vector(39 downto 0);
    signal w_OUTPUT_INST : std_logic_vector(31 downto 0);
    signal w_VALID_INST : std_logic := '0';
    signal r_RESET : std_logic := '0';
    signal current_clock : std_logic_vector (7 downto 0);
    signal current_instruction : std_logic_vector (31 downto 0);
    -- Component declaration for the Unit Under Test (UUT)
    component instruction_memory is
    generic (MAX_INSTRUCTION : natural);
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
        -- 50 bits total to store
        input_inst : in std_logic_vector(39 downto 0);
        -- The actual inst size is 50 - 8 (bit for clock) 32 bits
        output_inst : out std_logic_vector(31 downto 0);
        valid_inst : out std_logic :='0';
        reset : in std_logic
        );
    end component instruction_memory;

begin

    --Instantiate the Unit Under Test (UUT)
    UUT : instruction_memory
        generic map (MAX_INSTRUCTION => 7) -- meaning from 0 to 7 included (8 total) 
        port map (
        clock => r_CLOCK,
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

    p_CLK_GEN: process is
    begin
        wait for c_CLOCK_PERIOD/2;
        r_CLOCK <= not r_CLOCK;
    end process p_CLK_GEN;

    process
    begin 
        r_LOAD_INST <= '1';
        --current_clock <= x"05";
        --current_instruction <= x"55555555";
        r_INPUT_INST <= x"05" & x"55555555";
        r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_LOAD_INST <= '1';
        --current_clock <= x"07";
        --current_instruction <= x"77777777";
        r_INPUT_INST <=  x"07" & x"77777777";
        r_LOAD_NEXT_INST <= '0'; 
        wait for 50 ns;
        r_LOAD_INST <= '0';
        r_COMPUTING <= '1';
        wait for 50 ns;
        -- first computation clock
        wait for 50 ns;
        -- second computation clock
        wait for 50 ns;
        -- third computation clock
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
    end process;
end behave;




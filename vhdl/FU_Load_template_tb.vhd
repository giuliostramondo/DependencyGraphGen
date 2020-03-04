library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";

entity FU_Load_tb is
end FU_Load_tb;

architecture behave of FU_Load_tb is
    constant c_CLOCK_PERIOD : time := 50 ns;
    signal r_CLOCK: std_logic :='0';
    signal r_i_FU : std_logic_vector(32-1 downto 0);
    signal w_o_FU : std_logic_vector((32*3)-1 downto 0);
    signal r_r_COMPUTING : std_logic := '0';
    signal r_r_LOAD_INST : std_logic := '0';
    signal r_r_LOAD_NEXT_INST : std_logic := '0';
    signal r_r_INPUT_INST : std_logic_vector(16 downto 0);
    signal r_r_RESET : std_logic :='0';

    component FU_LOAD is
        generic (
                    INSTRUCTIONS : natural;
                    TOTAL_EXE_CYCLES : natural;
                    BITWIDTH : natural; -- 31 -> meaning 32
                    RF_DEPTH: natural; --15 -> meaning 16
                    OUTPUT_PORTS : natural
                );
        port(
            i_clock : in std_logic;
            i_FU : in std_logic_vector(BITWIDTH downto 0);
            o_FU : out std_logic_vector(((BITWIDTH +1)*OUTPUT_PORTS)-1 downto 0);
            r_COMPUTING : in std_logic;
            r_LOAD_INST: in std_logic := '0';
            r_LOAD_NEXT_INST: in std_logic ;
            r_INPUT_INST : in std_logic_vector(
                    natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ -- bit required to ID Clock
                    1 + -- Register Write enable bit 
                    natural(ceil(log2(real(RF_DEPTH)))) * 2  -- RF select bits (Read and Write addrs)
                    -1
                    downto 0);
            r_RESET : std_logic
            );
        end component FU_LOAD;
begin
    --instantiate the Unit Under Test (UUT)
    UUT: FU_LOAD
        generic map (
            INSTRUCTIONS => 7,
            BITWIDTH => 31,
            RF_DEPTH => 15,
            OUTPUT_PORTS => 3,
            TOTAL_EXE_CYCLES => 255
        )
        port map(
            i_clock => r_CLOCK,
            i_FU => r_i_FU,
            o_FU => w_o_FU,
            r_COMPUTING => r_r_COMPUTING,
            r_LOAD_INST => r_r_LOAD_INST,
            r_LOAD_NEXT_INST => r_r_LOAD_NEXT_INST,
            r_INPUT_INST => r_r_INPUT_INST,
            r_RESET => r_r_RESET
                );

    p_CLK_GEN: process is
    begin
        wait for c_CLOCK_PERIOD/2;
        r_CLOCK <= not r_CLOCK;
    end process p_CLK_GEN;
   
    process
    begin
        -- Write 3 elements from input in cycle 8,9,10 to registers 0,1,2
        -- Read 3 elements from registers 0,1,2 in cycle 11,12,13 
        r_r_LOAD_INST <= '1';
        --             @ cycle 8   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"08" &  "1"        & "0000"             & "0000" ;    
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --             @ cycle 9   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"09" &  "1"        & "0000"             & "0001" ;    
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --             @ cycle 10   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"0A" &  "1"        & "0000"             & "0010" ;    
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --             @ cycle 11   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"0B" &  "0"        & "0000"             & "0000" ;    
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --             @ cycle 12   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"0C" &  "0"        & "0001"             & "0000" ;    
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --             @ cycle 13   W enable  read RegAddress       write RegAddress
        r_r_INPUT_INST <= x"0D" &  "0"        & "0010"             & "0000" ;    
        r_r_LOAD_NEXT_INST <= '0';
        wait for 50 ns;
        -- first computation clock
        r_r_LOAD_INST <= '0';
        r_r_COMPUTING <= '1';
        wait for 50 ns;
        -- second computation clock
        wait for 50 ns;
        -- third computation clock
        wait for 50 ns;
        -- 4th computation clock
        wait for 50 ns;
        -- 5th computation clock
        wait for 50 ns;
        -- 6th computation clock
        wait for 50 ns;
        -- 7th computation clock
        wait for 50 ns;
        -- 8th computation clock
        --        Input 1  
        wait for 25 ns;
        r_i_FU <= x"00000001"; -- input for cycle 8
        wait for 50 ns;
        r_i_FU <= x"00000002"; -- input for cycle 9
        wait for 50 ns;
        r_i_FU <= x"00000003"; -- input for cycle 10
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
    end process; 
end behave;

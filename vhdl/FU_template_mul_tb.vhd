library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";

entity FU_template_mul_tb is
end FU_template_mul_tb;

architecture behave of FU_template_mul_tb is
    constant c_CLOCK_PERIOD : time := 50 ns;
    signal r_CLOCK: std_logic :='0';
    signal r_i_FU : std_logic_vector((32*4)-1 downto 0);
    signal w_o_FU : std_logic_vector((64*3)-1 downto 0);
    signal r_r_COMPUTING : std_logic := '0';
    signal r_r_LOAD_INST : std_logic := '0';
    signal r_r_LOAD_NEXT_INST : std_logic := '0';
    signal r_r_INPUT_INST : std_logic_vector(35 downto 0);
    signal r_r_RESET : std_logic :='0';
        --BEGIN DEBUG SIGNALS
    signal w_w_SEL_MUXA : std_logic_vector(1 downto 0);
    signal w_w_SEL_MUXB : std_logic_vector(1 downto 0);
    signal w_w_MUXA_OUT : std_logic_vector(63 downto 0);
    signal w_w_MUXB_OUT : std_logic_vector(63 downto 0);
    signal w_d_in_cb_out1_sel : natural range 0 to 4-1;
    signal w_d_in_cb_out2_sel : natural range 0 to 4-1;
    signal w_d_in_cb_out3_sel : natural range 0 to 4-1;
    signal w_d_input_crossbar_out1 : std_logic_vector(63 downto 0);
    signal w_d_input_crossbar_out2 : std_logic_vector(63 downto 0);
    signal w_d_input_crossbar_out3 : std_logic_vector(63 downto 0);
        --END DEBUG SIGNALS
    component FU is
        generic (
                    INSTRUCTIONS : natural;
                    TOTAL_EXE_CYCLES : natural;
                    BITWIDTH : natural; -- 31 -> meaning 32
                    RF_DEPTH: natural; --15 -> meaning 16
                    INPUT_PORTS : natural;
                    OUTPUT_PORTS : natural;
                    OPCODE :natural;
                    IS_MUL: natural
                );
        port(
            i_clock : in std_logic;
            --specify inputs (number of FUs that sends input to this FU)
           -- i_FU : in std_logic_vector(((BITWIDTH +1)*INPUT_PORTS)-1 downto 0);
            
            i_FU : in std_logic_vector(((BITWIDTH +1)*INPUT_PORTS)- 
            IS_MUL*((BITWIDTH +1)/2*INPUT_PORTS)   -1
            downto 0);
            --i_FU : in std_logic_vector(natural(real(IS_MUL)/real(2)*real(BITWIDTH +1)*real(INPUT_PORTS))-1 downto 0);
            --specify outputs (number of FUs that this FU can send the output to)
            o_FU : out std_logic_vector(((BITWIDTH +1)*OUTPUT_PORTS)-1 downto 0);
            -- SIGNALS FOR INSTRUCTION MEMORY
            r_COMPUTING : in std_logic;
            r_LOAD_INST: in std_logic := '0';
            r_LOAD_NEXT_INST: in std_logic ;
            r_INPUT_INST : in std_logic_vector(
                natural(ceil(log2(real(TOTAL_EXE_CYCLES+1))))+ -- bit required to ID Clock
                natural(ceil(log2(real(INPUT_PORTS))))*3 + -- crossbar Select Bits
                2 + -- Register Write enable bits (input 1 and input 2) 
                natural(ceil(log2(real(RF_DEPTH)))) * 4 + -- RF select bits
                4 -- Select signals for MuxA and MuxB
                -1
                downto 0);
            r_RESET : in std_logic

            );
    end component FU;
begin
    --instantiate the Unit Under Test (UUT)
    UUT: FU
        generic map (
            INSTRUCTIONS => 7,
            BITWIDTH => 63,
            RF_DEPTH => 15,
            INPUT_PORTS => 4,
            OUTPUT_PORTS => 3,
            TOTAL_EXE_CYCLES => 255,
            OPCODE => 1,
            IS_MUL => 1
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
        r_r_LOAD_INST <= '1';
        -- Select Input 1 and Input 2, perform operation on them
        --             @ cycle 8  cbOut 1 is input 1    cbOut2 is input 2  cbOut3 is input4
        r_r_INPUT_INST <= x"08" & "00"                  & "01"             & "11"       &    
        --   dont write to REG   r_REG1_s  r_REG2_s   w_REG1_s w_REG2_s  unused demuxes sel,  muxa and muxb sel cb in  
                          "00" & "0000"   & "0000"   & "0000" & "0000"       & "01"            & "01";
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        -- Use the output of the previous computation, forward it two both MuxA and MuxB and add them
        -- Store the result of the previous computation to REG2
        r_r_LOAD_INST <= '1';
        --             @ cycle 8  cbOut 1 is input 1    cbOut2 is input 2  cbOut3 is input3
        r_r_INPUT_INST <= x"09" & "00"                  & "01"             & "10"       &    
        --   dont write to REG   r_REG1_s  r_REG2_s   w_REG1_s w_REG2_s  unused demuxes sel,  muxa and muxb sel cb in  
                          "01" & "0000"   & "0000"   & "0000" & "0010"       & "00"            & "00";
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        -- Write the result of the previous computation to REG1, forward the result of the previous computation 
        -- To MuxA and MuxB and add them
        r_r_LOAD_INST <= '1';
        --             @ cycle 8  cbOut 1 is input 1    cbOut2 is input 2  cbOut3 is input3
        r_r_INPUT_INST <= x"0a" & "00"                  & "01"             & "10"       &    
        --   write to REG using input 2   r_REG1_s  r_REG2_s   w_REG1_s w_REG2_s  unused demuxes sel,  muxa and muxb sel cb in  
                          "01" & "0000"   & "0000"   & "0000" & "0001"      & "00"            & "00";
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        -- Prefetch value in REG1 for next cycle, forward the result of the previous computation To MuxA and MuxB and add them 
        r_r_LOAD_INST <= '1';
        --             @ cycle 8  cbOut 1 is input 1    cbOut2 is input 2  cbOut3 is input3
        r_r_INPUT_INST <= x"0b" & "00"                  & "01"             & "10"       &    
        --write to REG using input 2   r_REG1_s  r_REG2_s   w_REG1_s w_REG2_s  unused demuxes sel,  muxa and muxb sel cb in  
                                 "00" & "0001"   & "0001"   & "0000" & "0000"     & "00"            & "00";
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        -- Select Input 1 and send it to MuxA, send the output of the register file to MuxB 
        -- (it was prefetched the previous cycle), prefetch the value in register 2 and send it to MuxA and MuxB
        r_r_LOAD_INST <= '1';
        --             @ cycle 8  cbOut 1 is input 1    cbOut2 is input 2  cbOut3 is input3
        r_r_INPUT_INST <= x"0c" & "00"                  & "01"             & "10"       &    
        --   write to REG using input 2   r_REG1_s  r_REG2_s   w_REG1_s w_REG2_s  unused demuxes sel,  muxa and muxb sel cb in  
                          "00" & "0010"   & "0010"   & "0000" & "0000"  & "01"            & "10";
        r_r_LOAD_NEXT_INST <= '1';
        wait for 50 ns;
        r_r_LOAD_INST <= '1';
        --current_clock <= x"07";
        --current_instruction <= x"77777777";
        r_r_INPUT_INST <=  x"07" & x"7777777";
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
        --        Input 1        Input 2       Input 3        Input 4
        r_i_FU <= x"00000004" & x"00000003" & x"00000006" & x"00000001"; -- input for cycle 8
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
        wait for 50 ns;
       
    end process; 
end behave;

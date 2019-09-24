library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";

entity FU_register_file_tb is
end FU_register_file_tb;

architecture behave of FU_register_file_tb is
    constant c_CLOCK_PERIOD : time := 40 ns;
    signal r_CLOCK : std_logic := '0';
    signal r_RESET : std_logic := '0';
    signal r_INPUT_1 : std_logic_vector(31 downto 0);
    signal r_INPUT_2 : std_logic_vector(31 downto 0);
    signal r_WRITE_EN_1 : std_logic := '0';
    signal r_WRITE_EN_2 : std_logic := '0';
    signal r_REG1SEL_write : std_logic_vector(3 downto 0);
    signal r_REG2SEL_write : std_logic_vector(3 downto 0);
    signal r_REG1SEL_read : std_logic_vector(3 downto 0);
    signal r_REG2SEL_read : std_logic_vector(3 downto 0);
    signal w_OUTPUT_A : std_logic_vector(31 downto 0);
    signal w_OUTPUT_B : std_logic_vector(31 downto 0);


    -- Component declaration for the Unit Under Test (UUT)
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
    UUT : register_file 
        generic map (
            BITWIDTH => 31,
            DEPTH => 15
                    )
        port map (
        outA => w_OUTPUT_A,
        outB => w_OUTPUT_B,
        input1 => r_INPUT_1,
        input2 => r_INPUT_2,
        writeEnable1 => r_WRITE_EN_1,
        writeEnable2 => r_WRITE_EN_2,

        --the select signal needs to be log2(registerfilesize)
        reg1Sel_read  => r_REG1SEL_read,
        reg2Sel_read  => r_REG2SEL_read,
        reg1Sel_write  => r_REG1SEL_write,
        reg2Sel_write  => r_REG2SEL_write,
        clock => r_CLOCK,
        reset  => r_RESET
                 );

    p_CLK_GEN: process is
    begin
        wait for c_CLOCK_PERIOD/2;
        r_CLOCK <= not r_CLOCK;
    end process p_CLK_GEN;

    process
    begin 
        -- write using input 1 at location 0
        r_WRITE_EN_1 <='1';
        r_INPUT_1 <= x"00000001";
        r_REG1SEL_write <= "0000";
        wait for 80 ns;
        
        -- read using output A from location 0
        r_WRITE_EN_1 <='0';
        r_INPUT_1 <= x"00000000";
        r_REG1SEL_read <= "0000";
        wait for 80 ns;

        -- write using input 2 at location 1
        r_WRITE_EN_2 <='1';
        r_INPUT_2 <= x"00000002";
        r_REG2SEL_write <= "0001";
        wait for 80 ns;
        r_WRITE_EN_2 <='0';
        r_INPUT_2 <= x"00000000";
        r_REG2SEL_read <= "0001";
        wait for 80 ns;

        -- perform two parallel writes
        -- write 4 to reg 2
        -- write 5 to reg 3
        r_WRITE_EN_1 <='1';
        r_WRITE_EN_2 <='1';
        r_INPUT_1 <= x"00000004";
        r_INPUT_2 <= x"00000005";
        r_REG1SEL_write <= "0010";
        r_REG2SEL_write <= "0011";
        wait for 80 ns;
        -- now perform parallel read
        -- read from A reg 3 ( should give 5 in outA)
        -- read from B reg 2 (should give 4 in outB)
        r_WRITE_EN_1 <='0';
        r_WRITE_EN_2 <='0';
        r_INPUT_1 <= x"00000000";
        r_INPUT_2 <= x"00000000";
        r_REG1SEL_write <= "0000";
        r_REG2SEL_write <= "0000";
        r_REG1SEL_read <= "0011";
        r_REG2SEL_read <= "0010";
    end process;
end behave;




library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.math_real."ceil";
use IEEE.math_real."log2";

entity FU_register_file_paramIn_2Out_tb is
end FU_register_file_paramIn_2Out_tb;

architecture behave of FU_register_file_paramIn_2Out_tb is
    constant c_CLOCK_PERIOD : time := 50 ns;
    signal r_CLOCK : std_logic := '0';
    signal r_RESET : std_logic := '0';
    signal r_INPUT_1 : std_logic_vector(95 downto 0);
    signal r_INPUT_2 : std_logic_vector(31 downto 0);
    signal r_WRITE_EN_1 : std_logic_vector(2 downto 0);
    signal r_REG1SEL_write : std_logic_vector(11 downto 0);
    signal r_REG2SEL_write : std_logic_vector(3 downto 0);
    signal r_REG1SEL_read : std_logic_vector(3 downto 0);
    signal r_REG2SEL_read : std_logic_vector(3 downto 0);
    signal w_OUTPUT_A : std_logic_vector(31 downto 0);
    signal w_OUTPUT_B : std_logic_vector(31 downto 0);


    -- Component declaration for the Unit Under Test (UUT)
    component FU_register_file_paramIn_2Out is
        generic  
        (
            BITWIDTH : natural; --31
            DEPTH: natural; -- 15
            WRITE_LANES: natural -- 3
        );

        port
        (
        outA: out std_logic_vector(BITWIDTH downto 0);
        outB: out std_logic_vector(BITWIDTH downto 0);
        input1: in std_logic_vector(((BITWIDTH+1)*WRITE_LANES)-1 downto 0);
        writeEnable1: in std_logic_vector(WRITE_LANES-1 downto 0);

        --the select signal needs to be log2(registerfilesize)
        reg1Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
        reg2Sel_read : in std_logic_vector(natural(ceil(log2(real(DEPTH+1))))-1 downto 0);
        reg1Sel_write : in std_logic_vector((natural(ceil(log2(real(DEPTH+1))))*WRITE_LANES) -1 downto 0);
        clock : in std_logic;
        reset : in std_logic
        );
    end component FU_register_file_paramIn_2Out;
begin

    --Instantiate the Unit Under Test (UUT)
    UUT : FU_register_file_paramIn_2Out 
        generic map (
            BITWIDTH => 31,
            DEPTH => 15,
            WRITE_LANES => 3
                    )
        port map (
        outA => w_OUTPUT_A,
        outB => w_OUTPUT_B,
        input1 => r_INPUT_1,
        writeEnable1 => r_WRITE_EN_1,

        --the select signal needs to be log2(registerfilesize)
        reg1Sel_read  => r_REG1SEL_read,
        reg2Sel_read  => r_REG2SEL_read,
        reg1Sel_write  => r_REG1SEL_write,
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
        -- write using input 1 at location 1
        r_WRITE_EN_1 <= "100";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_write <= "000100100011";
        wait for 50 ns;
        
        -- read element at location 1
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0001";
        wait for 50 ns;

        -- write using input 2 at location 2
        r_WRITE_EN_1 <= "010";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_write <= "000100100011";
        wait for 50 ns;

        -- read using output A from location 2
        -- read using output B from location 1
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0010";
        r_REG2SEL_read <= "0001";
        wait for 50 ns;

        -- write using input 3 at location 3
        r_WRITE_EN_1 <= "001";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_write <= "000100100011";
        wait for 50 ns;

        -- read using output A from location 3
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0011";
        wait for 50 ns;

        -- write 3 inputs at the same time
        -- input 1 : 0x0BADF00D to reg 4
        -- input 2 : 0xDEADBEEF to reg 5
        -- input 3 : 0xDEAD2BAD to reg 6
        r_WRITE_EN_1 <= "111";
        r_INPUT_1 <= x"0BADF00DDEADBEEFDEAD2BAD";
        r_REG1SEL_write <= "010001010110";
        wait for 50 ns;

        -- read using output A from location 4
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0100";
        wait for 50 ns;

        -- read using output A from location 4
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0101";
        wait for 50 ns;


        -- read using output A from location 4
        r_WRITE_EN_1 <= "000";
        r_INPUT_1 <= x"000000010000000200000003";
        r_REG1SEL_read <= "0110";
        wait for 50 ns;
    end process;
end behave;




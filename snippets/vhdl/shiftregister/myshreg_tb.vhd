library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity myshreg_tb is
    
end myshreg_tb;

architecture myshreg_tb_behv of myshreg_tb is
   
   component myshreg
    Port (  input:      in std_logic;
              clock:      in std_logic;
              shift:      in std_logic;
              output:     out std_logic;
              dbg:        out std_logic_vector(3 downto 0));
   end component;

    signal T_input: std_logic;
    signal T_clock: std_logic;
    signal T_shift: std_logic;
    signal T_output: std_logic;
    signal T_dbg: std_logic_vector(3 downto 0);
    
begin
    U_shreg: myshreg port map(T_input, T_clock, T_shift, T_output, T_dbg);
    
    process
    begin
        T_clock <= '0';
        wait for 5 ns;
        T_clock <= '1';
        wait for 5 ns ;  
    end process;
    
    process
    begin
        T_shift <= '0';
        T_input <= '1';
        wait for 10 ns;
        T_shift <= '1';
        wait for 10 ns;
        T_shift <= '0';
        
        T_input <= '0';
        wait for 10 ns;
        T_shift <= '1';
        wait for 10 ns;
        T_shift <= '0';
    end process;
    
end myshreg_tb_behv;

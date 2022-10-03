----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 03/15/2020 11:44:21 AM
-- Design Name: 
-- Module Name: simple_testbench_tb - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use std.textio.all;
use IEEE.std_logic_textio.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity simple_testbench_tb is
--  Port ( );
end simple_testbench_tb;

architecture Behavioral of simple_testbench_tb is
    component simple_testbench is
        Port(
            a: in STD_LOGIC;
            b: in STD_LOGIC;
            c: in STD_LOGIC;
            j: out STD_LOGIC;
            K: out STD_LOGIC
        );
    end component;
    
    signal abc: std_logic_vector(2 downto 0);
    signal j,k: std_logic;
    
    signal debug_hidden_abc: std_logic_vector(2 downto 0);
    
    file fout: text;
begin
    UUT: simple_testbench port map ( a => abc(2), b => abc(1), c => abc(0), j => j, k => k );
    
    --Hierarchical references to signal
    --  enable VHDL 2008!
    debug_hidden_abc <= << signal UUT.hidden_abc: std_logic_vector(2 downto 0) >>;
    
    process 
        variable file_line: line;
        variable fstat: file_open_status;
    begin
        report "start my sim";
        --file_open(fstat, fout, "my_out_file.txt", write_mode);
        file_open(fstat, fout, "my_out_file.txt", append_mode);
        write(file_line, string'("abc "));
        write(file_line, abc);
        write(file_line, string'("; debug_hidden_abc "));
        write(file_line, debug_hidden_abc);
        writeline(fout, file_line);
        
        abc <= "000";
        wait for 100ns;
        report "testbench: 000 => 0 0";
        
        abc <= "001";
        wait for 100ns;
        
        abc <= "011";
        wait for 100ns;
        
        abc <= "111";
        wait for 100ns;
        
        file_close(fout);
    end process;

end Behavioral;

----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 04/13/2020 04:47:40 PM
-- Design Name: 
-- Module Name: sram_tb - Behavioral
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
use IEEE.NUMERIC_STD.ALL;
use std.textio.all;
use work.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity sram_tb is
--  Port ( );
end sram_tb;

architecture Behavioral of sram_tb is
    component sram is
        Port ( clk : in STD_LOGIC;
           write_en : in STD_LOGIC;
           address  : in STD_LOGIC_VECTOR(7 downto 0);
           datain   : in STD_LOGIC_VECTOR(7 downto 0);
           dataout  : out STD_LOGIC_VECTOR(7 downto 0) );
    end component;
    
    signal ADDR: std_logic_vector(7 downto 0);
    signal DATA: std_logic_vector(7 downto 0);
    signal DATA_READ: std_logic_vector(7 downto 0);
    signal WE: std_logic;
    signal CLK: std_logic;
    
    signal sram_content: project_types.ram_t;
    
    file fout: text;
begin
    UUT: sram PORT MAP(clk => CLK, write_en => WE, address => ADDR, datain => DATA, dataout => DATA_READ );
    sram_content <= << signal UUT.content: project_types.ram_t >>;

    process
        constant clk_half: time := 1ns;
        variable file_line: line;
        variable fstat: file_open_status;
    begin
        file_open(fstat, fout, "sram_tb_out.txt", write_mode);
       
         for iStep in 0 to 255 loop
            wait for clk_half; CLK <= '0';
      
            ADDR <= std_logic_vector( to_unsigned(iStep, ADDR'length) );
            DATA <= std_logic_vector( to_unsigned(iStep, ADDR'length) );
            WE <= '1';    
              
            wait for clk_half; CLK <= '1';
                    
            write(file_line, string'("write address=")); write(file_line, to_hstring(unsigned(ADDR)));
            write(file_line, string'("; data=")); write(file_line, to_hstring(unsigned(DATA)));
            writeline(fout, file_line);            
        end loop;   
        
        
        write(file_line, string'("---"));
        writeline(fout, file_line); 
        WE <= '0';
        for iStep in 0 to 255 loop
            
            wait for clk_half; CLK <= '0';
            
            ADDR <= std_logic_vector( to_unsigned(iStep, ADDR'length) );
            DATA <= "00000000";
            WE <= '0';    
            --------------------------------------------------------------------------------------
            wait for clk_half; CLK <= '1';
            wait for clk_half; CLK <= '0';
            wait for clk_half; CLK <= '1';
            --------------------------------------------------------------------------------------     
            write(file_line, string'("read address=")); write(file_line, to_hstring(unsigned(ADDR)));
            write(file_line, string'("; data=")); write(file_line, to_hstring(unsigned(DATA_READ)));
            writeline(fout, file_line);
            
        end loop;   
       
        std.env.finish;  
    end process;

end Behavioral;

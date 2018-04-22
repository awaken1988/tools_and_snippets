-- list of literature
--   http://vhdlguru.blogspot.de/2010/03/generics-in-vhdl-construction-of.html

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.myhelper.all;


entity mem_tb is
   
end mem_tb;


architecture behv of mem_tb is
    constant my_cell_count : natural :=256;

    component mem 
     generic (  DATAWIDTH: integer;
                CELLCOUNT: integer);
    port (  
     I_CLK: in std_logic;
     I_WE: in std_logic;
     I_ADDR: in std_logic_vector(addr_ld(my_cell_count)-1 downto 0);
     I_DATA: in std_logic_vector(8-1 downto 0);
     O_DATA: out std_logic_vector(8-1 downto 0)
    ); 
    end component;         

    signal I_CLK: std_logic;
    signal I_WE: std_logic;
    signal I_ADDR: std_logic_vector(addr_ld(my_cell_count)-1 downto 0);
    signal I_DATA: std_logic_vector(8-1 downto 0);
    signal O_DATA: std_logic_vector(8-1 downto 0);
    
    signal counter: integer := 0;
begin
    mem0: mem   generic map (DATAWIDTH => 8,  CELLCOUNT => my_cell_count) 
                port map(I_CLK, I_WE, I_ADDR, I_DATA, O_DATA);

    process
    begin
        I_CLK <= '1';
        wait for 5 ns;
        I_CLK <= '0';
        wait for 5 ns;
    end process;

    process(I_CLK)
    begin
       if counter < 10 then
           if falling_edge(I_CLK) then
            I_ADDR <= std_logic_vector( TO_UNSIGNED(counter, I_ADDR'length) );
            I_DATA <= std_logic_vector( TO_UNSIGNED(counter, I_DATA'length) );
            I_WE <= '1';
           end if;
           
           if rising_edge(I_CLK) then
            counter <= counter + 1;
           end if;
        else
            I_WE <= '0';
        end if;
    end process; 
end behv;
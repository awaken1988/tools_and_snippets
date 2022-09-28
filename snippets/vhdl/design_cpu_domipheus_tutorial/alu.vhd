-- source taken from: http://labs.domipheus.com/blog/designing-a-cpu-in-vhdl-part-4-alu-comparisons-branching/

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

use work.tpu_constants.all;

entity alu is Port (
    I_clk : in  STD_LOGIC;
    I_en :  in  STD_LOGIC;
    I_dataA :   in STD_LOGIC_VECTOR (15 downto 0);
    I_dataB :   in STD_LOGIC_VECTOR (15 downto 0);
    I_dataDwe : in STD_LOGIC;
    I_aluop :   in STD_LOGIC_VECTOR (4 downto 0);
    I_PC :      in STD_LOGIC_VECTOR (15 downto 0);
    I_dataIMM : in STD_LOGIC_VECTOR (15 downto 0);
    O_dataResult :   out  STD_LOGIC_VECTOR (15 downto 0);
    O_dataWriteReg : out STD_LOGIC;
    O_shouldBranch : out std_logic
  );
end alu;

architecture Behavioral of alu is
  -- The internal register for results of operations.
  -- 16 bit + carry/overflow
  signal s_result: STD_LOGIC_VECTOR(17 downto 0) := (others => '0');
  signal s_shouldBranch: STD_LOGIC := '0';
begin
  process (I_clk, I_en)
  begin
    if rising_edge(I_clk) and I_en = '1' then
      O_dataWriteReg <= I_dataDwe;
      case I_aluop(4 downto 1) is
        when OPCODE_ADD =>
          if I_aluop(0) = '0' then
            s_result(16 downto 0) <= std_logic_vector(unsigned('0' & I_dataA) + unsigned( '0' & I_dataB));
          else
            s_result(16 downto 0) <= std_logic_vector(signed(I_dataA(15) & I_dataA) + signed( I_dataB(15) & I_dataB));
          end if;
          s_shouldBranch <= '0';
 
        -- ...Other OPCODEs go here...
        when others =>
        s_result <= "00" & X"FEFE";
      end case;
    end if;
  end process;
 
  O_dataResult <= s_result(15 downto 0);
  O_shouldBranch <= s_shouldBranch;
 
end Behavioral;



local rounds = 100000000

local x = 1
local pi = 1.0

for i = 2, rounds + 2 do
    x = -x
    pi = pi + (x / (2 * i - 1))
end

pi = pi * 4

print(pi) 

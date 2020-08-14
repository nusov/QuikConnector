package.cpath = package.cpath .. ';' 
	.. getScriptPath() .. "\\lib\\x64\\Release\\?.dll"

require("QuikConnector")
require("QuikRouter")

isRun = true

function repl(x)
	local f = loadstring(x)
	if f == nil then
		return "SYNTAX ERROR"
	else
		return "RESULTS", f()
	end
end

function pull_trades(x)
	local rows = {}  
	local c = getNumberOf("all_trades")
	local i = x
	
	while i < c and i - x < 1000 do
		local r = getItem("all_trades", i)
		local d = r["datetime"]
		rows[i] = {
			ts         = os.time(d) + d["ms"] / 1000.0 + d["mcs"] / 1000000.0,
			class_code = r["class_code"],
			sec_code   = r["sec_code"],
			price      = r["price"],
			qty        = r["qty"],
			volume     = r["value"],
			flags      = r["flags"],
			period     = r["period"]
		}
		i = i + 1
	end

	return rows, i, getNumberOf("all_trades") - i
end

function main()
	rpc = QuikSocket.bind("tcp://0.0.0.0:8001")
	while isRun do
		rpc:process()
	end
end

function OnStop()
    isRun = false
end
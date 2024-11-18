
print("Welcome to GEXecutor primarily made by Now (@saqqwd) and http2 (@http2), you can get your name on the official executor too by making contributions.")
print("Git repo: https://gitea.com/Now/GEXecutor")
print(" ")

local _fetch_stubmodule do
	local current_module = 1
	local modules_list = {}
	local in_use_modules = {}
	
	for _, obj in game:FindService("CoreGui").RobloxGui.Modules:GetDescendants() do
		if not obj:IsA("ModuleScript") then
			if obj.Name:match("AvatarExperience") then
				for _, o in obj:GetDescendants() do
					if o.Name == "Flags" then
						for _, oa in o:GetDescendants() do
							if not oa:IsA("ModuleScript") then continue end
							table.insert(modules_list, oa:Clone())
						end
					elseif o.Name == "Test" then
						for _, oa in o:GetDescendants() do
							if not oa:IsA("ModuleScript") then continue end
							table.insert(modules_list, oa:Clone())
						end
					end
				end
			else
				if 
				obj.Name:match("ReportAnything") 
				or obj.Name:match("TestHelpers")
				--or obj.Name:match("Flags")
				then
					for _, o in obj:GetDescendants() do
						if not o:IsA("ModuleScript") then continue end
						table.insert(modules_list, o:Clone())
					end
				end
			end
				
			continue 
		end
	end
	
	local function find_new_module()
		local idx = math.random(1, #modules_list)
		while idx == current_module or in_use_modules[idx] do
			idx = math.random(1, #modules_list)
		end
		return idx
	end
	
	function _fetch_stubmodule()
		local idx = find_new_module()
	
		in_use_modules[current_module] = nil
		current_module = idx
		in_use_modules[current_module] = true
	
		return modules_list[idx]
	end
end
	
local fetch_stubmodule = _fetch_stubmodule


if script.Name == "JestGlobals" then
    local indicator = Instance.new("BoolValue")
    indicator.Name = "Exec"
    indicator.Parent = script

    local holder = Instance.new("ObjectValue")
    holder.Parent = script
    holder.Name = "Holder"
    holder.Value = fetch_stubmodule():Clone()
    print(holder.Value)

    local lsindicator = Instance.new("BoolValue")
    lsindicator.Name = "Loadstring"
    lsindicator.Parent = script

    local lsholder = Instance.new("ObjectValue")
    lsholder.Parent = script
    lsholder.Name = "LoadstringHolder"
    lsholder.Value = fetch_stubmodule():Clone()
    print(lsholder.Value)
end

--[[
if not game.CoreGui:FindFirstChild("Hi") then
    local rape = script:Clone()
    rape.Parent = game.CoreGui
    rape.Name = "Hi"

    wait(1)

    require(rape)
end


if not script.Name == "JestGlobals" then
    while true do
        wait(1)
    end
end
]]

local RunService = game:GetService("RunService")
if script.Name == "JestGlobals" then
    local exec = script.Exec
    local holder = script.Holder

local cooldownTime = 0.05
local lastExecutionTime = 0

task.spawn(function(...)
    RunService.RenderStepped:Connect(function()
        local currentTime = tick()  -- Get the current time in seconds
        if exec.Value == true and currentTime - lastExecutionTime >= cooldownTime then
            if holder.Value == nil and not notificationSent then
                notificationSent = true -- Set the flag to prevent multiple notifications
                game:GetService("StarterGui"):SetCore("SendNotification", {
                    Title = "gex",
                    Text = "Something went wrong while executing, try running the script again.",
                    Icon = ""
                })
                holder.Value = fetch_stubmodule():Clone()
            end

            local s, func = pcall(require, holder.Value)

            -- Reset holder for the next execution
            holder.Value = fetch_stubmodule():Clone()

            if s and type(func) == "function" then
                func()
            end

            exec.Value = false -- Reset exec value to false after execution
            notificationSent = false -- Reset the notification flag

            -- Update the last execution time to the current time
            lastExecutionTime = currentTime
        end
    end)
end)
end

wait() -- so recursive requires doesnt happen

-- This is support for many inject types, i highly suggest NOT using policyservice. << IMPORTANT

if script.Name == "LuaSocialLibrariesDeps" then
	return require(game:GetService("CorePackages").Packages.LuaSocialLibrariesDeps)
end
if script.Name == "JestGlobals" then
	return require(script)
end
if script.Name == "Url" then
	local a={}
	local b=game:GetService("ContentProvider")
	local function c(d)
		local e,f=d:find("%.")
		local g=d:sub(f+1)
		if g:sub(-1)~="/"then
			g=g.."/"
		end;
		return g
	end;
	local d=b.BaseUrl
	local g=c(d)
	local h=string.format("https://games.%s",g)
	local i=string.format("https://apis.rcs.%s",g)
	local j=string.format("https://apis.%s",g)
	local k=string.format("https://accountsettings.%s",g)
	local l=string.format("https://gameinternationalization.%s",g)
	local m=string.format("https://locale.%s",g)
	local n=string.format("https://users.%s",g)
	local o={GAME_URL=h,RCS_URL=i,APIS_URL=j,ACCOUNT_SETTINGS_URL=k,GAME_INTERNATIONALIZATION_URL=l,LOCALE_URL=m,ROLES_URL=n}setmetatable(a,{__newindex=function(p,q,r)end,__index=function(p,r)return o[r]end})
	return a
end
while wait(9e9) do wait(9e9);end

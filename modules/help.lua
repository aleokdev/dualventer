cmd_help = {
    name = "help",
    callback = function(ctx)
        local embed = {title="Command list", description="Here's the list of commands this bot has to offer:", fields = {}}
        for cmdname, command in pairs(ctx.bot.commands) do
            embed.fields[#embed.fields+1] = {}
            local field = embed.fields[#embed.fields]
            field["name"] = cmdname
            field["value"] = command.description or ""
            field["inline"] = false
        end
        ctx:send{embed=embed}
    end,

    description = "This command!",
    version = "1.0"
}

bot:add_command(cmd_help)
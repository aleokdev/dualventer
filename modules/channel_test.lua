cmd_channel_test = {
    name = "ctest",
    callback = function(ctx)
        local c = "Channel " .. tostring(ctx.channel.id) .. ":\n"
        c = c .. "Type: " .. tostring(ctx.channel.type) .. "\n"
        c = c .. "Position: " .. tostring(ctx.channel.position) .. "\n"
        c = c .. "Name: " .. tostring(ctx.channel.name) .. "\n"
        c = c .. "Topic: " .. tostring(ctx.channel.topic) .. "\n"
        c = c .. "NSFW: " .. tostring(ctx.channel.nsfw) .. "\n"
        c = c .. "Last Message ID: " .. tostring(ctx.channel.last_message_id) .. "\n"
        ctx:send{c}
    end,

    -- Any other fields will be ignored but can be used as metadata for other commands, such as help ones
    description = "Returns all available information about a channel",
    version = "1.0"
}

bot:add_command(cmd_channel_test)
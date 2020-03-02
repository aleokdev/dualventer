local cmd = {
    name = "test",
    callback = function(ctx)
        ctx:send{"Hello, this is a sample string"}
        ctx:send{"Hello, this is a sample string", embed={title="An embed", description="An embed too!"}}
        ctx:send{"The prefix of the bot is: "..ctx.bot.prefix}
        ctx:send{"The current channel ID is: "..ctx.channel.id}
        ctx:send{"The ID of the message that called the function is: "..ctx.message.id}
    end,

    -- Any other fields will be ignored but can be used as metadata for other commands, such as help ones
    description = "Tests the bot!",
    version = "1.0"
}

bot:add_command(cmd)
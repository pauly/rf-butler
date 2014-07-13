# RF Butler

Lightwaverf home automation 433mhz control for Arduino,
without relying on the wifi-link hardware.

Arduino home automation experiments. I have an arduino http://amzn.to/UCKWsq
with an ethernet shield, some LightwaveRF kit: http://amzn.to/RkukDo and
http://amzn.to/V7yPPK an rf transmitter and receiver http://bit.ly/HhltyI
and I'm trying to join them together.

So far it's only intercepting the messages sent by the rf transmitters, but soon I'll be sending messages too.

With thanks to https://github.com/roberttidey/LightwaveRF and https://github.com/lawrie/LightwaveRF

## So where does the tweeting come from?

Older versions of this script posted messages direct to the http://twitter.com/ourduino but that stopped working when twitter went all https. So now it posts to a proxy script which posts to the twitter. The script looks like this:

    #!/bin/env ruby
    require 'rubygems'
    require 'twitter'
    require 'cgi'
    cgi = CGI.new
    puts cgi.header
    puts '<html><body>'
    Twitter.configure do | config |
      config.consumer_key = "your consumer key"
      config.consumer_secret = cgi['c']
      config.oauth_token = "your oath token"
      config.oauth_token_secret = cgi['a']
    end
    tweet = cgi['t']
    # or build your own tweet out of the other params
    # $stderr.puts cgi.inspect
    # $stderr.puts tweet
    Twitter.update tweet
    puts 'ok'
    puts '</body></html>'

## Disclaimer

It's still not really a robot butler.

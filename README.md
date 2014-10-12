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

    #!/usr/bin/ruby
    require 'twitter'
    require "cgi"
    cgi = CGI.new
    $stderr.puts cgi.params.inspect
    client = Twitter::REST::Client.new do |config|
      config.consumer_key = "YOUR CONSUMER KEY"
      config.consumer_secret = cgi.params['c'][0]
      config.access_token = "YOUR ACCESS TOKEN"
      config.access_token_secret = cgi.params['a'][0]
    end
    cgi.out( 'status' => 'ok' ) { 'ok' }
    status = cgi.params['t'][0].to_s
    remote = cgi.params['re'][0].to_s
    target = cgi.params['ta'][0].to_s
    digital = cgi.params['di'][0].to_s
    analog = cgi.params['an'][0].to_s
    # add your own case statements here
    case remote
    when 'f3109'
      remote = 'white remote'
      case target
      when 'f1'
        target = 'everything'
      end
    end
    case digital
    when '0'
      digital = 'off'
    when '1'
      digital = 'on'
    end
    status = remote + ' turned ' + target + ' ' + digital + ' (' + analog + ')'
    client.update status

## Disclaimer

It's still not really a robot butler.

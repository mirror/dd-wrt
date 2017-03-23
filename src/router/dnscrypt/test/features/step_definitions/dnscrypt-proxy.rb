
require 'net/dns/resolver'

PROXY_IP = '127.0.0.1'
PROXY_PORT = 5300

Before do
  @resolver = Net::DNS::Resolver.new(nameserver: PROXY_IP, port: PROXY_PORT)
end

After do
  Process.kill("KILL", @pipe.pid) if @pipe
  @pipe = nil
end

Around do |scenario, block|
  Timeout.timeout(3.0) do
    block.call
  end
end

Given /^a running dnscrypt proxy with options "([^"]*)"$/ do |options|
  @pipe = IO.popen("dnscrypt-proxy " +
    "--local-address=#{PROXY_IP}:#{PROXY_PORT} $#{options}", "r")
  sleep(1.5)
end

When /^a client asks dnscrypt\-proxy for "([^"]*)"$/ do |name|
  @answer_section = @resolver.query(name, Net::DNS::A).answer
end

Then /^dnscrypt\-proxy returns "([^"]*)"$/ do |ip_for_name|
  expect(@answer_section.collect { |a| a.address.to_s }).to include(ip_for_name)
end

Then /^dnscrypt\-proxy returns a NXDOMAIN answer$/ do
  expect(@answer_section).to be_empty
end


require 'net/dns/resolver'

Given /^a working server proxy on (\d+\.\d+\.\d+\.\d+)$/ do |resolver|
  resolver = Net::DNS::Resolver.new(nameserver: resolver, port: 443)
  
  answer_section = resolver.query('2.dnscrypt-cert.fr.dnscrypt.org', Net::DNS::TXT).answer
  expect(answer_section).not_to be_empty
end

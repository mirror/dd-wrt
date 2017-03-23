require 'aruba/cucumber'
require 'net/dns'
ENV['PATH'] = "#{File.expand_path(File.dirname(__FILE__) + '/../../../src/dnscrypt-proxy')}#{File::PATH_SEPARATOR}#{ENV['PATH']}"

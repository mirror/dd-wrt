#! /usr/bin/env ruby

require 'date'
require 'pp'
require 'set'

REGEX = /(\w+)\s+'(.+)'\s+(.*)/

Commit = Struct.new(:hash, :time, :author, :stats)
CommitStats = Struct.new(:files, :nr_added, :nr_deleted)

def calc_stats(diff)
        changed = Set.new
        added = 0
        deleted = 0
        
        diff.lines.each do |l|
        	case l.encode('UTF-8', 'binary', invalid: :replace, undef: :replace, replace: '')
                when /^\+\+\+ (\S+)/
                        changed << $1
                when /^\+/
                        added = added + 1
                when /^---/
                        # do nothing
                when /^\-/
                        deleted = deleted + 1
        	end
	end

	CommitStats.new(changed, added, deleted)
end

def select_commits(&block)
        commits = []
        
	input = `git log --format="%h '%aI' %an"`
	input.lines.each do |l|
		m = REGEX.match(l)

		raise "couldn't parse: ${l}" unless m

		hash = m[1]
		time = DateTime.iso8601(m[2])
		author = m[3]

		if block.call(hash, time, author)
        		diff = `git log -1 -p #{hash} | filterdiff -X configure`
        		commits << Commit.new(hash, time, author, calc_stats(diff))
		end
	end
	
	commits
end

def since(date)
        lambda do |hash, time, author|
        	time >= date
        end
end

def pad(str, col)
        str + (' ' * (col - str.size))
end

def code_delta(s)
        s.nr_added + s.nr_deleted
end

def cmp_stats(lhs, rhs)
        code_delta(rhs) <=> code_delta(lhs)
end

#-----------------------------------

commits = select_commits(&since(DateTime.now - 14))

authors = Hash.new {|hash, key| hash[key] = CommitStats.new(Set.new, 0, 0)}

commits.each do |c|
	author_stats = authors[c.author]
	author_stats.files.merge(c.stats.files)
	author_stats.nr_added = author_stats.nr_added + c.stats.nr_added
	author_stats.nr_deleted = author_stats.nr_deleted + c.stats.nr_deleted
end

puts "#{pad("Author", 20)}\tChanged files\tInsertions\tDeletions"
authors.keys.sort {|a1, a2| cmp_stats(authors[a1], authors[a2])}.each do |k|
	v = authors[k]
	puts "#{pad(k, 20)}\t#{v.files.size}\t\t#{v.nr_added}\t\t#{v.nr_deleted}"
end

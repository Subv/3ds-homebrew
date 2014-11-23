codes = {}
File.readlines("output.txt").each do |line|
    data = line.split(" ")
    id = data[1]
    str = data[3]
    codes[id] = [str.to_i().to_s(16)].pack("H*").reverse
end

codes.sort_by { |code, str| code.to_i }.each do |code, str|
    puts "Code " + code + " String " + str
end

(0..0xBA).each do |i|
    if codes[i.to_s] != nil
        print "\"" + codes[i.to_s] + "\", "
    else
        print "nullptr, "
    end
    
    if (i + 1) % 8 == 0
        puts "// " + (i - 7).to_s + "-" + i.to_s
    end
end


def p_arr(arr)
    width = arr.flatten.max.to_s.size+2
    for i in (0..arr.length - 1)
        for j in (0..arr.length - 1)
            print arr[i][j].to_s.rjust(width + 2) + " "
        end
        puts ""
    end
end
def getrnd(range)
    rand -range.to_f..range.to_f
end

n = 8
arr = Array.new(2**n + 1) { Array.new(2**n +1) { 0 } }
n_augmented = 2**n 
arr[0][0] = rand(10 + 1)
# arr[0][n_augmented] = rand(10 + 1)
arr[0][n_augmented] = rand(10 + 1)
arr[n_augmented][0] = rand(10 + 1)
arr[n_augmented][n_augmented] = rand(10 + 1)
# arr[n_augmented][n_augmented] = 0


a = arr
n_augmented_copy = n_augmented
range = 50.0
while n_augmented > 1
    i = 0
    while(i < n_augmented_copy)
        j = 0
        while(j < n_augmented_copy)
            a[i][j+(n_augmented)/2] = (a[i][j] + a[i][j+n_augmented]) / 2.0  + getrnd(range)
            a[i + (n_augmented) / 2][j + n_augmented] = (a[i][j + n_augmented] + a[i + n_augmented][j + n_augmented]) / 2.0 + getrnd(range)
            a[i + n_augmented][j +( n_augmented) / 2] = (a[i + n_augmented][j] + a[i + n_augmented][j + n_augmented] )/ 2.0 + getrnd(range)
            a[i + (n_augmented) / 2][j]  = (a[i + n_augmented][j] + a[i][j] )/ 2.0 + getrnd(range)
            a[i + (n_augmented) / 2][j +( n_augmented) / 2] = ((a[i][j+(n_augmented)/2] + a[i + (n_augmented) / 2][j + n_augmented] + a[i + n_augmented][j +( n_augmented) / 2]  + a[i + (n_augmented) / 2][j]))/ 4.0  + getrnd(range)
            j = j + n_augmented    
        end
        i = i + n_augmented
    end
    range = range / 2
    n_augmented = n_augmented / 2
end
min_height = 100
max_height = 0
for i in 0..a.length - 1
    for j in 0..a.length - 1
        if a[i][j] < min_height
            min_height = a[i][j]
        end
        if a[i][j] >= max_height
            max_height = a[i][j]
        end
    end
end
board = n_augmented_copy + 1
c = 0 
k = board + 1
#Print vertices 
ridge_left = a.length/2 - 20 
ridge_right = a.length/2 + 20
diff = ridge_right - ridge_left
t = 0
count = 0
# A = 0
B = (min_height).to_f
tex = ["vt 0.0 0.0",
"vt 0.2 0.0",
"vt 0.2 0.2",
"vt 0.0 0.35",
"vt 0.2 0.35",
"vt 0.2 0.64",
"vt 0.36 0.66",
"vt 0.65 0.66",
"vt 0.65 0.99",
"vt 0.68 0.0",
"vt 0.88 0.0",
"vt 0.88 0.2",
"vt 0.1 0.68",
"vt 0.25 0.68",
"vt 0.25 0.88",
"vt 0.34 0.0",
"vt 0.64 0.0",
"vt 0.64 0.28",
"vt 0.68 0.68",
"vt 0.88 0.68",
"vt 0.88 0.88",
"vt 0.38 0.38",
"vt 0.62 0.38",
"vt 0.62 0.62"
]

File.open("models/mountain_rivers.obj", 'w') { |file| 
    for i in 0..a.length - 1
        t = 0
        roller = 0.04
        last = 0
        range2 = 5.0
        # A = (a[i][ridge_left] / 1.2).to_f
        # B = min_height.to_f
        for j in 0..a.length - 1
            if (j <= ridge_left) || (j >= ridge_right)
                file.write("v " + j.to_f.to_s + " " + a[i][j].to_f.to_s + " " + i.to_f.to_s+ "\n")
            else
                if j >=ridge_left && j <= (ridge_left + ridge_right)/2
                    t = t + roller
                    roller -= 0.002
                    a[i][j] = (1 - t) * ((a[i][ridge_left] / 1.2).to_f) + t * B
                    last = a[i][j] - rand(range2)
                    file.write("v " + j.to_f.to_s + " " + (last).to_f.to_s + " " + i.to_f.to_s+ "\n")
                    range2 = range2 /2
                else
                    file.write("v " + j.to_f.to_s + " " + (last).to_f.to_s + " " + i.to_f.to_s+ "\n")

                end
            end
        end
    end

    for t in 0..tex.length - 1
        file.write(tex[t] + "\n")
    end

    for i in 0..a.length - 2
        for j in 0..a.length - 2
            # file.write("f " + (c + j + 1).to_s  + " " + k.to_s + " " + (c + j + 2).to_s + "\n")
            # file.write("f " + (c + j + 2).to_s + " "  + (k).to_s + " " + (k+1).to_s  + "\n")
            if (j <= ridge_left + 5) || (j >= ridge_right -5)
                # if a[i][j] >= min_height && a[i][j] <= max_height / 10
                if a[i][j] >= min_height && a[i][j] <= min_height + 20 
                    file.write("f " + (c + j + 1).to_s  + "/16/ " + k.to_s + "/17/ " + (c + j + 2).to_s + "/18/\n")
                    file.write("f " + (c + j + 2).to_s + "/16/ "  + (k).to_s + "/17/ " + (k+1).to_s  + "/18/\n")
                elsif a[i][j] > min_height + 20 && a[i][j] <= max_height / 6
                    file.write("f " + (c + j + 1).to_s  + "/19/ " + k.to_s + "/20/ " + (c + j + 2).to_s + "/21/\n")
                    file.write("f " + (c + j + 2).to_s + "/19/ "  + (k).to_s + "/20/ " + (k+1).to_s  + "/21/\n")
                elsif a[i][j] > max_height / 6 && a[i][j] <= max_height / 4
                    file.write("f " + (c + j + 1).to_s  + "/1/ " + k.to_s + "/2/ " + (c + j + 2).to_s + "/3/\n")
                    file.write("f " + (c + j + 2).to_s + "/1/ "  + (k).to_s + "/2/ " + (k+1).to_s  + "/3/\n")
                elsif a[i][j] > max_height / 4 && a[i][j] <= max_height / 2
                    file.write("f " + (c + j + 1).to_s  + "/7/ " + k.to_s + "/8/ " + (c + j + 2).to_s + "/9/\n")
                    file.write("f " + (c + j + 2).to_s + "/7/ "  + (k).to_s + "/8/ " + (k+1).to_s  + "/9/\n")
                elsif a[i][j] > max_height / 2 && a[i][j] <= max_height / 1.5
                    file.write("f " + (c + j + 1).to_s  + "/10/ " + k.to_s + "/11/ " + (c + j + 2).to_s + "/12/\n")
                    file.write("f " + (c + j + 2).to_s + "/10/ "  + (k).to_s + "/11/ " + (k+1).to_s  + "/12/\n")
                elsif a[i][j] >= max_height / 1.5
                    file.write("f " + (c + j + 1).to_s  + "/13/ " + k.to_s + "/14/ " + (c + j + 2).to_s + "/15/\n")
                    file.write("f " + (c + j + 2).to_s + "/13/ "  + (k).to_s + "/14/ " + (k+1).to_s  + "/15/\n")
                end
            else
                # if rand(10) < 7
                    file.write("f " + (c + j + 1).to_s  + "/4/ " + k.to_s + "/5/ " + (c + j + 2).to_s + "/6/\n")
                    file.write("f " + (c + j + 2).to_s + "/4/ "  + (k).to_s + "/5/ " + (k+1).to_s  + "/6/\n")
                # elsif rand(10) >= 7 && rand(10) < 8
                #     file.write("f " + (c + j + 1).to_s  + "/22/ " + k.to_s + "/23/ " + (c + j + 2).to_s + "/24/\n")
                #     file.write("f " + (c + j + 2).to_s + "/22/ "  + (k).to_s + "/23/ " + (k+1).to_s  + "/24/\n")
                # elsif rand(10) >= 8
                #     file.write("f " + (c + j + 1).to_s  + "/13/ " + k.to_s + "/14/ " + (c + j + 2).to_s + "/15/\n")
                #     file.write("f " + (c + j + 2).to_s + "/13/ "  + (k).to_s + "/14/ " + (k+1).to_s  + "/15/\n")  
                # end
            end
            k=k+1
        end
        k = k +1
        c += board 
    end
}

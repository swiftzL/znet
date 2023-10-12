print("init main lua file")
function OnInit(id)
    print("lua [main] init id:"..id)
    test()
end
function test(a,b,c)
    a = 100
    b = 200
    c = 300
    local d = "fsdfafa";
    print(d)
    znet.Write(a,b,c,4,5,6)
end
function OnExit()
    print("lua [main] exit")
end
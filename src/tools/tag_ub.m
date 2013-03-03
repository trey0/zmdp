
n = 29;
gamma = 0.95;

V = 0;
for i=0:500
  if i == n
    break
  end
  p = 1/(i+1);
  %p = 1/n;
  V = 10*p + (1-p) * (-1 + gamma*V);
end
V

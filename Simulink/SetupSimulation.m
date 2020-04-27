clear all
close all

% for a given step, equivalent plant transfer function is LR series circuit
% with series BEMF independent source

% phase voltage -> current transfer function
% I = V/(R + sL)

% discrete time equivalent is found using Bilinear Transform:

% I     1         s^-1          b_0 + b_1 s^-1
% - = -----  = -----------  =  -----------------
% V   R + sL   R s^-1 + L       a_0 + a_1 s^-1

% K = 2/Ts
R = 1; %ohm
L = 0.005; %henry
tau = L/R;

fs = 50000000; % simulate at 50MHz
Ts = 1/fs;

K = 2/Ts;

b_0 = 0;
b_1 = 1;
a_0 = L;
a_1 = R;

% bilinear transform: 

B_0 = b_0 * K + b_1;
B_1 = -1 * b_0 * K + b_1;
A_0 = a_0 * K + a_1;
A_1 = -1 * a_0 * K + a_1;

% express transfer function as difference equation for simulation

% y[n] = C_1 x[n] + C_2 x[n-1] - D_1 y[n-1]

C_1 = B_0/A_0;
C_2 = B_1/A_0;
D_1 = A_1/A_0;

%--------------------------------------------------------------------------%

count_max = 500-1;
temp = [0:count_max];
fliptemp = flip(temp);
triangle_vals = [temp fliptemp(1,2:(count_max+1))];


triangle_times = Ts.*[1:(2*count_max+1)];

Tpwm = Ts * (2*count_max+1);

%--------------------------------------------------------------------------%

test_bemf = 1;

iref = 1; % 1 A reference current

Vsupply = 12; 

%Vsupply * D * Tpwm + test_bemf * (1-D) * Tpwm  = Vavg = I ref * R

test_fixed_duty = (iref * R + test_bemf) / Vsupply;% should be such that average current is iref
test_fixed_duty_comp = ceil(test_fixed_duty * count_max);

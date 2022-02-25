clc 
clear

fileID = fopen('C:\Users\Nikolas\Desktop\rtes\Latency.bin');

format = 'int';
A = fread(fileID, Inf, format);
tSpan = 0:0.1:25199.9;
tSpan = tSpan';

maxValue = max(A)
minValue = min(A)
meanValue = mean(A)
standardDeviation = std(A)
variance = var(A)

figure(1)

h = stem(A);
set(h, 'MarkerFaceColor', 'blue', 'Marker', 'none')
xlabel('time');
ylabel('delay (\mus)');
xlim([0 25200]);

j = 1;
for i = 1 : size(A)
    if A(i) < 300 && A(i) > 50
        B(j) = A(i);
        j = j + 1;
    end
end

figure(2)
histogram(B);
xlabel('\mus');
ylabel('# of appearance');
grid on;

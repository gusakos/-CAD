% ================================================================== %
%                                                                    %
%               All of this code was generated by ChatGPT            %
%                                                                    %
% ================================================================== %

% Define the polynomial function
f = @(x) 3*x.^5 - 124*x.^4 + 439*x.^3 + 1234*x.^2 + 10439*x - 931678;

% Define the range of x-values to plot
x = linspace(-1000, 1000, 10000);

% Evaluate the polynomial function for each x-value
y = f(x);

% Plot the polynomial function
plot(x, y);

% Add axis labels and a title
xlabel('x');
ylabel('f(x)');
title('Plot of f(x) = 3x^5 -124x^4+439x^3+1234x^2+10439x-931678');

% Plot the x-axis as a black line
hold on;
line([min(x), max(x)], [0, 0], 'color', 'black');

hold off;

% Define the coefficients of the polynomial
c = [3, -124, 439, 1234, 10439, -931678];

% Find all of the roots of the polynomial
r = roots(c);

% Filter out the complex roots
real_roots = r(imag(r) == 0);

% Display the real roots
disp(real_roots);

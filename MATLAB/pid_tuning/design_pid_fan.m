function pid_fan = design_pid_fan(model_fan)

model_fan_inv = -model_fan;

pid_fan = pidtune(model_fan_inv, 'PID');

disp('--- Regulator PID wentylatora ---');
disp(pid_fan);

end

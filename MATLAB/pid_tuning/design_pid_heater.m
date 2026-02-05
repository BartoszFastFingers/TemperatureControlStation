function pid_heater = design_pid_heater(model_heater)

pid_heater = pidtune(model_heater, 'PID');

disp('--- Regulator PID grzałki ---');
disp(pid_heater);

end

function simulate_pid(plant, pid_ctrl, name)

sys_cl = feedback(pid_ctrl * plant, 1);

figure;
step(sys_cl, 5000);
grid on;
title(['Odpowiedź skokowa – ' name]);
ylabel('Temperatura [°C]');
xlabel('Czas [s]');

end

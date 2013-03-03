function v = plot_prec(probname, probdesc)

history = load([probname,'_history.plot']);
precision = history(:,1);
regret = history(:,2);
comp_time = history(:,3);
dummy = history(:,4);

n = length(precision)-1;

h = semilogy(comp_time,precision,'-', comp_time,regret,'--');
%h = loglog(comp_time,precision,'-', comp_time,regret,'--');

title(probdesc)
xlabel('computation time (seconds)')
%ylabel('width of interval')
legend(h, 'precision', 'regret')

v = axis;

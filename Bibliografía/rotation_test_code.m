% Rotación de prueba (Grados)
orient = 0;     % Orientación
cab = 91;        % Cabeceo
rol = 0;        % Roleo

disp('Angulos ingresados (Grados):')
disp([orient,cab,rol]);

% Convenciones
th1 = -orient;
th2 = cab;
th3 = rol;
% De la teoría
ph1 = -th1;
ph2 = -th2;
ph3 = -th3;
R = rotz(ph1)*rotx(ph2)*roty(ph3);

% Yo digo que R es algo que se puede estimar en la placa en base al campo
%magnético y el acelerómetro, si g no es paralelo a B.

% Magia negra y trigonometría (no pregunten, es un copy pase sofisticado)
rol_rec = atan2( R(3,1), R(3,3) );
if(abs(R(3,2)) > 1)
    R(3,2) = sign(R(3,2));
end
cab_rec = asin( -R(3,2) );
orient_rec = -atan2( R(1,2), R(2,2) );

% "Orientación" +-180
% "Rolido" +-180
% "Cabeceo" +- 90 degrees
% Parece lo más lógico (y me dio asi, ya fue).
% Consigna pide Rolido +-180, cabeceo +-180
% Pero hay que limitar a 90, dado que caso contrario
% [O R C] == [0 180 180] == [180 0 0]
% Segunda opción es la correcta, dado que refleja orientación de la placa.
disp('Angulos recuperados (Grados):')
op1 = rad2deg([orient_rec,cab_rec,rol_rec]);
disp(rad2deg([orient_rec,cab_rec,rol_rec]));
alt1 = rad2deg([orient_rec,-cab_rec,rol_rec])+[180,180,-180];
alt2 = rad2deg([orient_rec,-cab_rec,rol_rec])+[-180,180,+180];

% El único válido es el 1ro. Las alternativas permiten detectar
% cámbios de ángulo de forma correcta (es necesario?)

if(~exist('last'))
    last = op1;
end

diff_1 = op1-last;  %Distancia 1
diff_2 = alt1-last; %Distancia 2
diff_3 = alt2-last; %Distancia 3
diff_i = [diff_1; diff_2; diff_3];

%La distancia correcta es la de menor norma
norms = [norm(diff_1),norm(diff_2),norm(diff_3)];
minim = min(norms);
[~,i] = find(norms==minim);
disp('Diferencia (Grados):')
disp(diff_i(i,:));

last = op1;
                              
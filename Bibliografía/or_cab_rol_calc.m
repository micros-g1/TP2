% This will fail if B is parallel to G (won't happen)

% Measured B, G
B = [0, 0, 1]'; G = [0, 1, 0]'; % Expected = [0, -90, 0]
% B = [0, 1, 0]' ; G = [0, 0, -1]'; % Expected = [0,0,0]
% B = [0, 1, -0.5]' ; G = [0, 0, -1]'; % Expected = [0,0,0]
% B = [0, 0, -1]' ; G = [0, -1, 0]'; % Expected = [0, 90, 0]
% B = [0, -1, 0]' ; G = [0, 0, 1]'; % Expected = [180, 0, 180]

% G measurement is accurate. get Up versor
% Sensor may actually measure -g !!
u = -G/norm(G);
%B tends to point to the north - magnetic south. Remove projection of B over u
% and normalize to compute estimation of North versor
n = B - dot(B,u)*u;
n = n/norm(n);
%Right hand rule. Compute East versor
e = cross(n,u);
% compute R (actually, we don't need it, we can replace in the formula...)
R = [e,n,u];

% Magia negra y trigonometría (no pregunten, es un copy pase sofisticado)
rol_rec = atan2( R(3,1), R(3,3) );
if(abs(R(3,2)) > 1)
    R(3,2) = sign(R(3,2));
end
cab_rec = asin( -R(3,2) );
orient_rec = -atan2( R(1,2), R(2,2) );

%Show
disp(rad2deg([orient_rec,cab_rec,rol_rec]));